import 'dart:async';
import 'dart:convert';
import 'dart:developer' show log;
import 'dart:io' as io;
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:google_sign_in/google_sign_in.dart';
import 'package:google_sign_in_platform_interface/google_sign_in_platform_interface.dart' as gsi;
import 'package:googleapis/drive/v3.dart' as drive;
import 'package:http/http.dart' as http;
import 'package:lowresrmx/data/google_auth_client.dart';
import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:cloud_sync_core/cloud_sync_core.dart';
import 'package:cloud_sync_drive/cloud_sync_drive.dart';
import 'package:path/path.dart' as p;
import 'package:crypto/crypto.dart';
import 'package:lowresrmx/data/sync_adapter.dart';

class SyncManager with ChangeNotifier {
  SyncManager() {
    _init();
  }

  static const List<String> googleScopes = [drive.DriveApi.driveFileScope];

  late Future<void> _signInInitialized;
  GoogleSignInAccount? _currentUser;
  String? _accountEmail;
  bool _authorized = false;
  bool _unavailable = false;
	bool _syncing = false;

  /// False when google_sign_in has no implementation on this platform (Linux,
  /// Windows) or initialization failed; every entry point is then a no-op.
  bool get isAvailable => !_unavailable;

  /// True when Drive calls work right now: the scopes are granted and the
  /// platform hands out access tokens without any user interaction.
  bool get isAuthorized => _authorized;

  /// The connected account, remembered across launches, or null when no
  /// account was ever connected.
  String? get accountEmail => _accountEmail;

  /// The account's display name. Only an interactive sign-in reports it, so a
  /// session restored at launch has the address and nothing else.
  String? get accountName => _currentUser?.displayName;

	bool get syncing => _syncing;

  void _init() {
    final GoogleSignIn signIn = GoogleSignIn.instance;

    signIn.authenticationEvents.listen((event) async {
      if (event is GoogleSignInAuthenticationEventSignIn) {
        _remember(event.user);
        await _checkAuthorization();
      } else if (event is GoogleSignInAuthenticationEventSignOut) {
        _forget();
      }
    });

    _signInInitialized = signIn
        .initialize(
          // It is safe
          clientId:
              "204783433847-laro4ojkci5oriqqv956dp2n4pjigpuu.apps.googleusercontent.com",
          serverClientId:
              "204783433847-3m9hqdqcofo8lsj1bqkh3bm2upa4kvjh.apps.googleusercontent.com",
        )
        .then((_) async {
      // Nothing authenticates at launch: attemptLightweightAuthentication
      // opens a "choose a saved sign-in" sheet whenever Credential Manager
      // cannot auto-select. It is not needed either — the grant lives in the
      // platform, not in this app, so the remembered address is enough to pick
      // access tokens back up silently.
      _accountEmail = await MyPreference.getGoogleAccount();
      if (_accountEmail != null) {
        _authorized = await _accessToken() != null;
      }
      notifyListeners();
    }).catchError((Object err) {
      debugPrint("Google sign-in unavailable: $err");
      _unavailable = true;
      notifyListeners();
    });
  }

  /// Signs in when no account is connected yet and asks for the Drive scopes.
  /// Shows UI: call it from a user gesture only. True when Drive is usable.
  Future<bool> connect() async {
    if (_unavailable) return false;
    await _signInInitialized;
    try {
      GoogleSignInAccount? user = _currentUser;
      if (user == null) {
        user =
            await GoogleSignIn.instance.authenticate(scopeHint: googleScopes);
        _remember(user);
      }
      await user.authorizationClient.authorizeScopes(googleScopes);
      _authorized = true;
    } on GoogleSignInException catch (err) {
      // Includes the ordinary "user cancelled" outcome.
      debugPrint("Google connect failed: $err");
    }
    notifyListeners();
    return _authorized;
  }

  /// Revokes the Drive grant and forgets the account.
  Future<void> disconnect() async {
    if (_unavailable) return;
    try {
      await GoogleSignIn.instance.disconnect();
    } on GoogleSignInException catch (err) {
      debugPrint("Google disconnect failed: $err");
    }
    _forget();
  }

  /// An HTTP client that carries a fresh access token on every request.
  /// The caller owns it and should [http.Client.close] it when done.
  http.Client authorizedClient() => GoogleAuthClient(headers: _authHeaders);

  /// Drive API over [authorizedClient]. Build one per batch of work; the
  /// underlying client is not closed for you.
  drive.DriveApi driveApi() => drive.DriveApi(authorizedClient());

  void _remember(GoogleSignInAccount user) {
    _currentUser = user;
    _accountEmail = user.email;
    MyPreference.setGoogleAccount(user.email);
  }

  void _forget() {
    _currentUser = null;
    _accountEmail = null;
    _authorized = false;
    MyPreference.setGoogleAccount(null);
    notifyListeners();
  }

  Future<void> _checkAuthorization() async {
    _authorized = await _accessToken() != null;
    notifyListeners();
  }

  /// A currently valid access token for [googleScopes], or null when only user
  /// interaction could produce one. Never shows UI.
  ///
  /// This goes through the platform interface rather than
  /// `GoogleSignInAccount.authorizationClient` so that it works with no
  /// authenticated user — the grant belongs to the app and the account, not to
  /// a session — and so that the account address reaches the platform, which
  /// needs it to revoke the grant in [disconnect].
  Future<String?> _accessToken() async {
    final gsi.ClientAuthorizationTokenData? tokens =
        await gsi.GoogleSignInPlatform.instance.clientAuthorizationTokensForScopes(
      gsi.ClientAuthorizationTokensForScopesParameters(
        request: gsi.AuthorizationRequestDetails(
          scopes: googleScopes,
          userId: _currentUser?.id,
          email: _accountEmail,
          promptIfUnauthorized: false,
        ),
      ),
    );
    return tokens?.accessToken;
  }

  Future<Map<String, String>?> _authHeaders() async {
    final String? token = await _accessToken();
    if (token == null) {
      if (_authorized) {
        _authorized = false;
        notifyListeners();
      }
      return null;
    }
    return <String, String>{'Authorization': 'Bearer $token'};
  }

  static Future<String> _hashFile(io.File file) async =>
      sha256.convert(await file.readAsBytes()).toString();

  /// Every syncable file in the library, keyed by bare file name.
  ///
  /// The Drive adapter is a flat store keyed by the Drive file *name*, so a
  /// manifest key is a file name, never a device path — absolute paths do not
  /// survive a reinstall or another device.
  Future<Map<String, SyncFileEntry>> _libraryManifest() async {
    final io.Directory libraryDir = await MyLibrary.getLibraryDir();
    final Map<String, SyncFileEntry> files = {};
    await for (final entry in libraryDir.list()) {
      if (entry is! io.File) continue;
      final String name = p.basename(entry.path);
      if (!_isSyncable(name)) continue;
      files[name] = SyncFileEntry(
        path: name,
        sha256: await _hashFile(entry),
        lastModified: await entry.lastModified(),
      );
    }
    return files;
  }

  static bool _isSyncable(String name) {
    if (name.isEmpty || name.startsWith(".") || name.contains("/")) return false;
    final String extension = p.extension(name);
    return extension == MyLibrary.codeExtension ||
        extension == MyLibrary.thumbExtension;
  }

  Future<void> _syncFiles(
    Map<String, SyncFileEntry> files, {
    required bool full,
  }) async {
    final io.Directory libraryDir = await MyLibrary.getLibraryDir();
    final http.Client client = authorizedClient();
    final CachedDriveAdapter adapter = CachedDriveAdapter(
      DriveAdapter.appFiles(httpClient: client, folderName: 'Game Creator'),
    );
    int localChanges = 0;
    try {
      await adapter.load();
      await adapter.ensureFolder();
      await _migrateLegacyPaths(adapter);
      if (full) {
        localChanges += await _propagateDeletions(adapter, files, libraryDir);
      }

      final SyncEngine engine = SyncEngine(adapter: adapter);
      final SyncResult result = await engine.sync(
        localPath: libraryDir.path,
        localManifest: SyncManifest(files: files, lastSynced: DateTime.now()),
        direction: full ? SyncDirection.bidirectional : SyncDirection.push,
        readLocalFile: (name) =>
            io.File(p.join(libraryDir.path, name)).readAsBytes(),
        writeLocalFile: (name, bytes) async {
          if (!_isSyncable(name)) return;
          final io.File file = io.File(p.join(libraryDir.path, name));
          if (p.extension(name) == MyLibrary.thumbExtension) {
            await FileImage(file).evict();
          }
          await file.writeAsBytes(bytes);
          localChanges += 1;
        },
      );

      log("sync: full=$full up=${result.filesUploaded} "
          "down=${result.filesDownloaded} errors=${result.errors}");

      if (full && result.errors.isEmpty) {
        await MyPreference.setSyncBaseline(jsonEncode(SyncManifest(
          files: await _libraryManifest(),
          lastSynced: result.syncedAt,
        ).toJson()));
      }
    } on GoogleAuthorizationRequired {
      _authorized = false;
      log("sync: authorization lost");
    } catch (error) {
      log("sync failed: $error");
    } finally {
      try {
        await adapter.finish();
      } catch (error) {
        log("sync: hash cache not saved: $error");
      }
      client.close();
      _syncing = false;
      notifyListeners();
      if (localChanges > 0) MyLibrary().refresh();
    }
  }

  /// Renames the Drive files an earlier build named after full device paths.
  /// Runs once, before anything else touches the folder.
  Future<void> _migrateLegacyPaths(CachedDriveAdapter adapter) async {
    if (await MyPreference.getSyncPathMigrated()) return;
    final Map<String, RemoteFileInfo> listing = await adapter.listFiles();
    for (final String key in listing.keys.toList()) {
      if (!key.contains("/")) continue;
      final String name = key.substring(key.lastIndexOf("/") + 1);
      if (!_isSyncable(name)) continue;
      final RemoteFileInfo? target = listing[name];
      if (target == null ||
          listing[key]!.lastModified.isAfter(target.lastModified)) {
        await adapter.uploadFile(name, await adapter.downloadFile(key));
      }
      await adapter.deleteFile(key);
      log("sync: migrated $key -> $name");
    }
    await MyPreference.setSyncPathMigrated();
  }

  /// Applies deletions that happened on either side since the last clean full
  /// sync. Returns the number of local files removed.
  Future<int> _propagateDeletions(
    CachedDriveAdapter adapter,
    Map<String, SyncFileEntry> files,
    io.Directory libraryDir,
  ) async {
    final String? raw = await MyPreference.getSyncBaseline();
    if (raw == null) return 0;
    final SyncManifest baseline =
        SyncManifest.fromJson(jsonDecode(raw) as Map<String, dynamic>);
    final Map<String, RemoteFileInfo> remote =
        await adapter.listFilesWithHashes();
    int removed = 0;

    for (final MapEntry<String, SyncFileEntry> entry in baseline.files.entries) {
      final String name = entry.key;
      final SyncFileEntry base = entry.value;
      final SyncFileEntry? local = files[name];
      final RemoteFileInfo? peer = remote[name];

      if (local == null && peer != null) {
        // Deleted here. Only delete remotely when the remote copy is still the
        // one we last saw; otherwise another device changed it and the engine
        // pulls it back.
        if (peer.sha256 == base.sha256) {
          await adapter.deleteFile(name);
          log("sync: deleted remote $name");
        }
      } else if (local != null && peer == null) {
        // Deleted elsewhere. Only delete locally when this copy is unmodified
        // since the baseline; otherwise the engine uploads it again.
        if (local.sha256 == base.sha256) {
          final io.File file = io.File(p.join(libraryDir.path, name));
          if (p.extension(name) == MyLibrary.thumbExtension) {
            await FileImage(file).evict();
          }
          if (await file.exists()) await file.delete();
          files.remove(name);
          removed += 1;
          log("sync: deleted local $name");
        }
      }
    }
    return removed;
  }

  Future<void> syncProgram(String programName) async {
    if (!_canSync || _syncing) return;
    _syncing = true;
    notifyListeners();

    final Map<String, SyncFileEntry> files = {};
    for (final io.File file in [
      await MyLibrary.getCodeFile(programName),
      await MyLibrary.getThumbFile(programName),
    ]) {
      if (!await file.exists()) continue;
      final String name = p.basename(file.path);
      if (!_isSyncable(name)) continue;
      files[name] = SyncFileEntry(
        path: name,
        sha256: await _hashFile(file),
        lastModified: await file.lastModified(),
      );
    }
    if (files.isEmpty) {
      _syncing = false;
      notifyListeners();
      return;
    }
    await _syncFiles(files, full: false);
  }

  Future<void> syncAllPrograms() async {
    if (!_canSync || _syncing) return;
    _syncing = true;
    notifyListeners();
    await _syncFiles(await _libraryManifest(), full: true);
  }

  bool get _canSync => isAvailable && isAuthorized && accountEmail != null;
}

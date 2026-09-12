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
import 'package:cryptography/cryptography.dart';
import 'package:shared_preferences/shared_preferences.dart';

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

	// 	final authClient = DriveAuthClient({
  //   	'Authorization': 'Bearer YOUR_ACCESS_TOKEN_HERE',
  // 	});

	// 	final adapter = DriveAdapter.appFiles(
  //   httpClient: authClient,
  //   folderName: 'MyApp',
  //   subPath: 'Backups',
  // );
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

	Future<SyncEngine> getSyncEngine() async {
		final String? token = await _accessToken();
		final authClient = DriveAuthClient({
    	'Authorization': "Bearer $token"
  	});
		final adapter = DriveAdapter.appFiles(
			httpClient: authClient,
			folderName: 'Game Creator'
		);
		final engine = SyncEngine(adapter: adapter);
		return engine;
	}

	Future<String> _hashFromFile(io.File file) async {
		// final hashPath = "${file.path}.sha256";
		// final hashFile = io.File(hashPath);
		final algorithm = Sha256();
		final hash=await algorithm.hash(await file.readAsBytes());
		return base64Encode(hash.bytes);
	}

	Future<void> _syncFiles(Map<String, SyncFileEntry> files) async {
		final prefs = await SharedPreferences.getInstance();

		final lastSynced = prefs.getString("lastSynced");
		log("lastSynced: $lastSynced");
		final manifest = SyncManifest(
			files: files,
			lastSynced: lastSynced!=null ? DateTime.parse(lastSynced) : DateTime.now()
		);

		final engine = await getSyncEngine();

		final result = await engine.sync(
			localPath: '/ignored-since-callbacks-are-explicit',
			localManifest: manifest,
			direction: SyncDirection.bidirectional,
			readLocalFile: (path) async => io.File(path).readAsBytes(),
			writeLocalFile: (path, bytes) async {
				final extension = p.extension(path);
				if (p.basename(path)[0]==".") return;
				if (extension == MyLibrary.codeExtension || extension == MyLibrary.thumbExtension)
				{
					io.File(path).writeAsBytes(bytes);
				}
			},
		);

		prefs.setString("lastSynced",result.syncedAt.toIso8601String());

		log('Sync success: ${result.success}');
		log('Sync conflict: ${result.conflicts}');
		log('Sync uploaded: ${result.filesUploaded}');
		log('Sync downloaded: ${result.filesDownloaded}');
		log('Sync deleted: ${result.filesDeleted}');
		log('Sync errors: ${result.errors.length}');
		log(result.errors.toString());

		// TODO: handle conflict

		_syncing = false;
		notifyListeners();
	}

	Future<void> syncProgram(String programName) async {
		_syncing = true;
		notifyListeners();

		if (!isAvailable) return;
		if (!isAuthorized) return;
		if (accountEmail == null) return;

		final codeFile = await MyLibrary.getCodeFile(programName);
		final thumbFile = await MyLibrary.getThumbFile(programName);

		Map<String, SyncFileEntry> files={};
		if (await codeFile.exists())
		{
			files[codeFile.path] = SyncFileEntry(
				path: codeFile.path,
				sha256: await _hashFromFile(codeFile),
				lastModified: await codeFile.lastModified());
		}
		if (await thumbFile.exists())
		{
			files[thumbFile.path] = SyncFileEntry(
				path: thumbFile.path,
				sha256: await _hashFromFile(thumbFile),
				lastModified: await thumbFile.lastModified());
		}

		_syncFiles(files);
	}

	Future<void> syncAllPrograms() async {
		_syncing = true;
		notifyListeners();

		if (!isAvailable) return;
		if (!isAuthorized) return;
		if (accountEmail == null) return;

		final io.Directory libraryDir = await MyLibrary.getLibraryDir();

		Map<String, SyncFileEntry> files={};
		await for(var entry in libraryDir.list())
		{
			final extension = p.extension(entry.path);
			if (entry is! io.File) continue;
			if (p.basename(entry.path)[0]==".") continue;
			if (extension == MyLibrary.codeExtension || extension == MyLibrary.thumbExtension)
			{
				final file = io.File(entry.path);
				files[entry.path] = SyncFileEntry(
					path: entry.path,
					sha256: await _hashFromFile(file),
					lastModified: await file.lastModified());
			}
		}

		_syncFiles(files);
	}

}

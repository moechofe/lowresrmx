import 'dart:convert';

import 'package:cloud_sync_core/cloud_sync_core.dart';
import 'package:cloud_sync_drive/cloud_sync_drive.dart';
import 'package:crypto/crypto.dart';
import 'package:lowresrmx/data/preference.dart';

class _HashEntry {
  const _HashEntry(
      {required this.modified, required this.size, required this.hash});

  final String modified;
  final int size;
  final String hash;

  Map<String, dynamic> toJson() => {'m': modified, 's': size, 'h': hash};

  factory _HashEntry.fromJson(Map<String, dynamic> json) => _HashEntry(
        modified: json['m'] as String,
        size: json['s'] as int,
        hash: json['h'] as String,
      );
}

/// Wraps [DriveAdapter] so the engine stops re-downloading the whole folder.
///
/// Drive reports md5, not sha256, so a sha256 computed once is remembered
/// against the file's (modifiedTime, size) and served back through
/// [RemoteFileInfo.sha256]; the engine only downloads on a miss. Bytes fetched
/// or uploaded during one run are also kept in memory so the hash pass and the
/// transfer pass share a single download.
class CachedDriveAdapter implements StorageAdapter {
  CachedDriveAdapter(this._inner);

  final DriveAdapter _inner;
  final Map<String, List<int>> _content = {};
  final Map<String, _HashEntry> _cache = {};
  Map<String, RemoteFileInfo> _listing = {};
  bool _listed = false;

  /// The listing from the last [listFiles] call. The engine lists before it
  /// transfers anything, so a pull can read the remote timestamp from here.
  Map<String, RemoteFileInfo> get lastListing => _listing;

  Future<void> load() async {
    final raw = await MyPreference.getSyncHashCache();
    if (raw == null) return;
    final decoded = jsonDecode(raw) as Map<String, dynamic>;
    decoded.forEach((name, value) {
      _cache[name] = _HashEntry.fromJson(value as Map<String, dynamic>);
    });
  }

  /// Re-lists once when anything moved, so bytes uploaded this run get their
  /// fresh (modifiedTime, size) recorded, then persists the pruned cache.
  Future<void> finish() async {
    if (_content.isNotEmpty) {
      _listing = await _inner.listFiles();
      _listed = true;
      _content.forEach(_remember);
    }
    // A run that failed before ever listing saw no remote state; pruning
    // against an empty listing would throw the whole cache away.
    if (!_listed) return;
    _cache.removeWhere((name, _) => !_listing.containsKey(name));
    await MyPreference.setSyncHashCache(
      jsonEncode(_cache.map((name, entry) => MapEntry(name, entry.toJson()))),
    );
  }

  /// Listing with every sha256 filled, downloading what is not cached.
  Future<Map<String, RemoteFileInfo>> listFilesWithHashes() async {
    final listing = await listFiles();
    final result = <String, RemoteFileInfo>{};
    for (final entry in listing.entries) {
      final info = entry.value;
      if (info.sha256 != null) {
        result[entry.key] = info;
        continue;
      }
      final bytes = await downloadFile(entry.key);
      result[entry.key] = RemoteFileInfo(
        path: info.path,
        lastModified: info.lastModified,
        sizeBytes: info.sizeBytes,
        sha256: sha256.convert(bytes).toString(),
      );
    }
    return result;
  }

  void _remember(String name, List<int> bytes) {
    final info = _listing[name];
    if (info == null) return;
    if (info.sizeBytes != 0 && info.sizeBytes != bytes.length) return;
    _cache[name] = _HashEntry(
      modified: info.lastModified.toIso8601String(),
      size: info.sizeBytes,
      hash: sha256.convert(bytes).toString(),
    );
  }

  @override
  Future<void> ensureFolder() => _inner.ensureFolder();

  @override
  Future<Map<String, RemoteFileInfo>> listFiles() async {
    _listing = await _inner.listFiles();
    _listed = true;
    final result = <String, RemoteFileInfo>{};
    _listing.forEach((name, info) {
      final entry = _cache[name];
      final fresh = entry != null &&
          entry.modified == info.lastModified.toIso8601String() &&
          entry.size == info.sizeBytes;
      if (!fresh) _cache.remove(name);
      result[name] = RemoteFileInfo(
        path: info.path,
        lastModified: info.lastModified,
        sizeBytes: info.sizeBytes,
        sha256: fresh ? entry.hash : null,
      );
    });
    return result;
  }

  @override
  Future<List<int>> downloadFile(String remotePath) async {
    final cached = _content[remotePath];
    if (cached != null) return cached;
    final bytes = await _inner.downloadFile(remotePath);
    _content[remotePath] = bytes;
    _remember(remotePath, bytes);
    return bytes;
  }

  @override
  Future<void> uploadFile(String remotePath, List<int> content) async {
    await _inner.uploadFile(remotePath, content);
    _content[remotePath] = content;
    _cache.remove(remotePath); // modifiedTime unknown until finish() re-lists
  }

  @override
  Future<void> deleteFile(String remotePath) async {
    await _inner.deleteFile(remotePath);
    _content.remove(remotePath);
    _cache.remove(remotePath);
    _listing.remove(remotePath);
  }
}

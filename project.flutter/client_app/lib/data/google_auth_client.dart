import 'package:http/http.dart' as http;

/// Supplies `Authorization` headers for the requested Google scopes, or null
/// when the grant is missing and only user interaction could restore it.
typedef GoogleAuthHeaders = Future<Map<String, String>?> Function();

/// Thrown when a request needs a Google authorization that cannot be obtained
/// without user interaction.
class GoogleAuthorizationRequired implements Exception {
  const GoogleAuthorizationRequired();

  @override
  String toString() =>
      'GoogleAuthorizationRequired: the requested Google scopes are not granted';
}

/// An [http.Client] that asks for authorization headers on every request.
///
/// Google access tokens live about an hour, and the native sign-in SDKs expose
/// neither an expiry nor a refresh token, so a captured token can never be
/// renewed. Reading the current token per request lets the platform hand back a
/// fresh one once the old one expired.
class GoogleAuthClient extends http.BaseClient {
  GoogleAuthClient({required GoogleAuthHeaders headers, http.Client? inner})
      : _headers = headers,
        _inner = inner ?? http.Client(),
        _ownsInner = inner == null;

  final GoogleAuthHeaders _headers;
  final http.Client _inner;
  final bool _ownsInner;

  @override
  Future<http.StreamedResponse> send(http.BaseRequest request) async {
    final Map<String, String>? headers = await _headers();
    if (headers == null) {
      throw const GoogleAuthorizationRequired();
    }
    request.headers.addAll(headers);
    return _inner.send(request);
  }

  @override
  void close() {
    if (_ownsInner) {
      _inner.close();
    }
    super.close();
  }
}

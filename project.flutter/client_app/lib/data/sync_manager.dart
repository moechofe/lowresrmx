import 'dart:async';
import 'package:flutter/foundation.dart';
import 'package:google_sign_in/google_sign_in.dart';
import 'package:googleapis/drive/v3.dart';
import 'package:lowresrmx/data/preference.dart';

class SyncManager with ChangeNotifier {
	late Future<void> _signInInitialized;
  GoogleSignInAccount? _currentUser;
  bool _isAuthorized = false;
  GoogleSignInAccount? get currentUser => _currentUser;
  bool get isLoggedIn => _currentUser != null;
  bool get isAuthorized => _isAuthorized;

  static const List<String> googleScopes = [DriveApi.driveFileScope];

  SyncManager() {
    _init();
  }

  void _init() {
    final GoogleSignIn signIn = GoogleSignIn.instance;
		_signInInitialized = signIn.initialize(
			// TODO: hide this
			clientId: "204783433847-laro4ojkci5oriqqv956dp2n4pjigpuu.apps.googleusercontent.com",
			serverClientId: "204783433847-3m9hqdqcofo8lsj1bqkh3bm2upa4kvjh.apps.googleusercontent.com",
		);

    signIn.authenticationEvents.listen((event) async {
      if (event is GoogleSignInAuthenticationEventSignIn) {
        _currentUser = event.user;
        _isAuthorized = (await _currentUser?.authorizationClient.authorizationForScopes(googleScopes)) != null;
      } else if (event is GoogleSignInAuthenticationEventSignOut) {
        _currentUser = null;
        _isAuthorized = false;
      }
      notifyListeners();
    });

		_signInInitialized.then((value) async {
			if (await MyPreference.getGoogleSigned()) {
				signIn.attemptLightweightAuthentication();
			}
		});
  }

  Future<void> login() async {
    try {
      await GoogleSignIn.instance.authenticate();
    } catch (err) {
      debugPrint("Login error: $err");
    }
  }

  Future<void> logout() async {
    await GoogleSignIn.instance.disconnect();
  }

  Future<void> requestDrivePermissions() async {
    if (_currentUser == null) return;
    _isAuthorized = await _currentUser?.authorizationClient.authorizeScopes(googleScopes) != null;
    notifyListeners();
  }
}

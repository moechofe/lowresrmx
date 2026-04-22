import 'dart:async';
import 'package:flutter/foundation.dart';
import 'package:google_sign_in/google_sign_in.dart';
import 'package:extension_google_sign_in_as_googleapis_auth/extension_google_sign_in_as_googleapis_auth.dart';
import 'package:googleapis/drive/v3.dart';
import 'package:lowresrmx/data/preference.dart';

class SyncManager with ChangeNotifier {
	late Future<void> _signInInitialized;
  GoogleSignInAccount? _currentUser;
	GoogleSignInClientAuthorization? _authorization;

  static const List<String> googleScopes = [DriveApi.driveFileScope];

  SyncManager() {
    _init();
  }

  void _init() {
    final GoogleSignIn signIn = GoogleSignIn.instance;
		_signInInitialized = signIn.initialize(
			// It is safe
			clientId: "204783433847-laro4ojkci5oriqqv956dp2n4pjigpuu.apps.googleusercontent.com",
			serverClientId: "204783433847-3m9hqdqcofo8lsj1bqkh3bm2upa4kvjh.apps.googleusercontent.com",
		);

    signIn.authenticationEvents.listen((event) async {
      if (event is GoogleSignInAuthenticationEventSignIn) {
        _currentUser = event.user;
				_requestAuthorization();
      } else if (event is GoogleSignInAuthenticationEventSignOut) {
        _currentUser = null;
				_authorization = null;
				MyPreference.setGoogleSigned(false);
				notifyListeners();
      }
    });

		_signInInitialized.then((value) async {
			if (await MyPreference.getGoogleSigned()) {
				signIn.attemptLightweightAuthentication();
				_checkAuthorization();
			}
		});
  }

  Future<void> login() async {
    try {
      // authenticate() replaces the old signIn() method.
      await GoogleSignIn.instance.authenticate();
    } catch (err) {
      debugPrint("Login error: $err");
    }
  }

  Future<void> logout() async {
    await GoogleSignIn.instance.disconnect();
  }

	void _updateAuthorization(GoogleSignInClientAuthorization? authorization) {
		_authorization = authorization;

		if (authorization != null) {
			MyPreference.setGoogleSigned(true);
			notifyListeners();
		}
	}

  Future<void> _checkAuthorization() async {
		_updateAuthorization(
			await _currentUser?.authorizationClient.authorizationForScopes(googleScopes));
	}

	Future<void> _requestAuthorization() async {
		_updateAuthorization(await _currentUser?.authorizationClient.authorizeScopes(googleScopes));
	}

  // /// Returns a [DriveApi] instance if the user is logged in and authorized.
  // Future<DriveApi?> getDriveApi() async {
  //   final account = _currentUser;
  //   if (account == null) return null;

  //   // Ensure we have the necessary scopes
  //   final isAuthorized = (await account.authorizationClient.authorizationForScopes(googleScopes)) != null;
  //   if (!isAuthorized) return null;

  //   final client = await account.authClient();
  //   if (client == null) return null;
  //   return DriveApi(client);
  // }
}

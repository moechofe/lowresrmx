<?php

require_once __DIR__.'/common.php';

if(preg_match('/^\/google$/',$urlPath)&&$isGet)
{
	error_log(__FILE__);
	// Sauce: https://developers.google.com/identity/protocols/oauth2/web-server

	$sequence=redis()->incr('seq:google');
	$google_state=sodium_crypto_shorthash(strval($sequence),LOGIN_GOOGLE_KEY);

	// If a upload token is passed, the user is using the iOS app to share a program.
	$uptoken=@$_GET['uptoken'];
	redis()->set("l:$google_state",$uptoken,"ex",LOGIN_GOOGLE_TTL);

	$config=json_decode(file_get_contents("https://accounts.google.com/.well-known/openid-configuration"),true);

	$auth_request=$config['authorization_endpoint'].'?'.http_build_query([
		'client_id'=>GOOGLE_CLIENT_ID,
		'redirect_uri'=>WEBSITE_URL."{$urlPath}",
		'response_type'=>'code',
		'scope'=>'openid profile',
		'access_type'=>'offline',
		'state'=>bin2hex($google_state),
	]);
	error_log("Auth request: $auth_request");

	header("Location: $auth_request");
	exit;
}

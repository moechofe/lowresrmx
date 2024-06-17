<?php

require_once __DIR__.'/common.php';

if($url['path']==='/google')
{
	// Sauce: https://developers.google.com/identity/protocols/oauth2/web-server

	$sequence=redis()->incr('seq:google');
	$google_state=sodium_crypto_shorthash(strval($sequence),LOGIN_GOOGLE_KEY);

	redis()->setex("l:$google_state",LOGIN_GOOGLE_TTL,$google_state);

	// TODO: hide the user-agent
	$config=json_decode(file_get_contents("https://accounts.google.com/.well-known/openid-configuration"),true);

	$auth_request=$config['authorization_endpoint'].'?'.http_build_query([
		'client_id'=>GOOGLE_CLIENT_ID,
		'redirect_uri'=>WEBSITE_URL.'/google',
		'response_type'=>'code',
		'scope'=>'openid profile',
		'access_type'=>'offline',
		'state'=>bin2hex($google_state),
	]);

	header("Location: $auth_request");
	exit;
}

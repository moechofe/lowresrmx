<?php

require_once __DIR__.'/common.php';

if(preg_match('/^\/google$/',$urlPath)&&$isGet&&!empty($_GET['code'])&&!empty($_GET['state']))
{
	error_log(__FILE__);

	$state=hex2bin($_GET['state']);
	if(!redis()->exists("l:$state")) forbidden("Fail to validate state");

	// If a upload token is passed, the user is using the iOS app to share a program.
	$uptoken=redis()->get("l:$state");
	redis()->del("l:$state");

	// Sauce: https://developers.google.com/identity/protocols/oauth2/web-server

	$config=json_decode(file_get_contents("https://accounts.google.com/.well-known/openid-configuration"),true);

	$token_request=['http'=>[
		'method'=>'POST',
		'header'=>'Content-Type: application/x-www-form-urlencoded',
		'content'=>http_build_query([
			'client_id'=>GOOGLE_CLIENT_ID,
			'client_secret'=>GOOGLE_CLIENT_SECRET,
			'code'=>$_GET['code'],
			'grant_type'=>'authorization_code',
			'redirect_uri'=>WEBSITE_URL.$url['path'],
		])
	]];
	error_log("Token request: ".json_encode($token_request));
	$token_response=json_decode(file_get_contents($config['token_endpoint'],false,stream_context_create($token_request)),true);
	error_log("Token response: ".json_encode($token_response));
	if(empty($token_response['access_token'])) internalServerError("Fail to read access token");

	$profile_request=['http'=>[
		'method'=>'GET',
		'header'=>"Authorization: {$token_response['token_type']} {$token_response['access_token']} \r\n"
	]];
	error_log("Profile request: ".json_encode($profile_request));
	$profile_response=json_decode(file_get_contents("https://www.googleapis.com/userinfo/v2/me",false,stream_context_create($profile_request)),true);
	error_log("Profile response: ".json_encode($profile_response));
	if(empty($profile_response['id'])) internalServerError("Fail to read google id");

	$user_id="{$profile_response['id']}.go2";

	$session_id=""
	.sodium_crypto_shorthash($token_response['access_token'],SESSION_GOOGLE_KEY)
	// .sodium_crypto_shorthash($token_response['refresh_token'],SESSION_GOOGLE_KEY)
	.sodium_crypto_shorthash($profile_response['id'],SESSION_GOOGLE_KEY);

	redis()->hmset("s:$session_id",
		"uid",$user_id,
		"status","allowed",
		"ct",date(DATE_ATOM)
	);
	redis()->expire("s:$session_id",SESSION_TTL);

	redis()->rpush("u:$user_id:s",$session_id);

	redis()->hset("u:$user_id","name",@$profile_response['given_name']);
	redis()->hset("u:$user_id","picture",@$profile_response['picture']);
	redis()->hsetnx("u:$user_id","author",@$profile_response['given_name']);

	redis()->hsetnx("u:$user_id:g","locale",@$profile_response['locale']?:'en');

	$session_id=bin2hex($session_id);

	// If a upload token is passed, the user is using the iOS app to share a program.
	if($uptoken) $location='/share?uptoken='.$uptoken;
	else $location='/community.html';
	error_log("Location: $location");

	header("Set-Cookie: ".join("; ",[
		HEADER_SESSION_COOKIE."=$session_id",
		// "SameSite=Strict",
		// "Secure",
		// "HttpOnly", // XXX: Prevent to see the cookie in the browser
		// "Path=/",
		"Max-Age=".SESSION_TTL,
		// "Domain=".DOMAIN,
	]));
	header("Location: $location");
	exit;

// 	echo <<<HTML
// <!DOCTYPE html>
// <html lang="en">
// <head>
// <script>
// window.localStorage.setItem('session','$session_id');
// window.location.replace('$location');
// </script>
// </head>
// </html>
// HTML
// 	;
// 	exit;
}

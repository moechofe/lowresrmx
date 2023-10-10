<?php

require_once __DIR__.'/common.php';

if($url['path']==='/google'&&!empty($_GET['code'])&&!empty($_GET['state']))
{
	$state=hex2bin($_GET['state']);

	if(redis()->get("l:$state")!==$state) forbidden("Fail to validate state");

	redis()->del("l:$state");

	// Sauce: https://developers.google.com/identity/protocols/oauth2/web-server

	$config=json_decode(file_get_contents("https://accounts.google.com/.well-known/openid-configuration"),true);

	$token_request=stream_context_create(['http'=>[
		'method'=>'POST',
		'header'=>'Content-Type: application/x-www-form-urlencoded',
		'content'=>http_build_query([
			'client_id'=>GOOGLE_CLIENT_ID,
			'client_secret'=>GOOGLE_CLIENT_SECRET,
			'code'=>$_GET['code'],
			'grant_type'=>'authorization_code',
			'redirect_uri'=>WEBSITE_URL.'/google',
		])
	]]);
	$token_response=json_decode(file_get_contents($config['token_endpoint'],false,$token_request),true);
	if(empty($token_response['access_token'])) internalServerError("Fail to read access token");

	$profile_request=stream_context_create(['http'=>[
		'method'=>'GET',
		'header'=>"Authorization: {$token_response['token_type']} {$token_response['access_token']} \r\n"
	]]);
	$profile_response=json_decode(file_get_contents("https://www.googleapis.com/userinfo/v2/me",false,$profile_request),true);
	if(empty($profile_response['id'])) internalServerError("Fail to read google id");

	$user_id="{$profile_response['id']}.go2";

	$session_id=""
	.sodium_crypto_shorthash($token_response['access_token'],SESSION_GOOGLE_KEY)
	.sodium_crypto_shorthash($token_response['refresh_token'],SESSION_GOOGLE_KEY)
	.sodium_crypto_shorthash($profile_response['id'],SESSION_GOOGLE_KEY);

	redis()->hmset("s:$session_id",
		"uid",$user_id,
		"status","allowed",
		"ct",date(DATE_ATOM)
	);

	redis()->rpush("u:$user_id:s",$session_id);

	redis()->hsetnx("u:$user_id","name",$profile_response['given_name']);
	redis()->hsetnx("u:$user_id","picture",$profile_response['picture']);

	redis()->hsetnx("u:$user_id","locale",$profile_response['locale']);

	$session_id=bin2hex($session_id);

	echo <<<HTML
<!DOCTYPE html>
<html lang="en">
<head>
<script>
window.localStorage.setItem('session','$session_id');
window.location.replace('/community.html');
</script>
</head>
</html>
HTML
	;
	exit;
}

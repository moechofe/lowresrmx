<?php

require_once __DIR__.'/common.php';

if($url['path']==='/discord'&&!empty($_GET['code'])&&!empty($_GET['state']))
{
	$state=hex2bin($_GET['state']);

	if(redis()->get("l:$state")!==$state) forbidden("Fail to validate state");

	redis()->del("l:$state");

	// Sauce: https://discord.com/developers/docs/topics/oauth2#shared-resources-oauth2-urls

	$token_request=stream_context_create(['http'=>[
		'method'=>'POST',
		'header'=>'Content-Type: application/x-www-form-urlencoded',
		'content'=>http_build_query([
			'client_id'=>DISCORD_CLIENT_ID,
			'client_secret'=>DISCORD_CLIENT_SECRET,
			'code'=>$_GET['code'],
			'grant_type'=>'authorization_code',
			'redirect_uri'=>WEBSITE_URL.'/discord',
		])
	]]);
	error_log("Token request: ".json_encode($token_request));
	$token_response=json_decode(file_get_contents("https://discord.com/api/oauth2/token",false,$token_request),true);
	error_log("Token Response: ".json_encode($token_response));
	if(empty($token_response['access_token'])) internalServerError("Fail to read access token");

	$profile_request=stream_context_create(['http'=>[
		'method'=>'GET',
		'header'=>"Authorization: {$token_response['token_type']} {$token_response['access_token']} \r\n"
	]]);
	error_log("Profile request: ".json_encode($profile_request));
	$profile_response=json_decode(file_get_contents("https://discord.com/api/users/@me",false,$profile_request),true);
	error_log("Profile response: ".json_encode($profile_response));

	// TODO: check something

	$user_id="{$profile_response['id']}.di2";

	$session_id=""
	.sodium_crypto_shorthash($token_response['access_token'],SESSION_DISCORD_KEY)
	.sodium_crypto_shorthash($token_response['refresh_token'],SESSION_DISCORD_KEY)
	.sodium_crypto_shorthash($profile_response['id'],SESSION_DISCORD_KEY);

	redis()->hmset("s:$session_id",
		"uid",$user_id,
		"status","allowed",
		"ct",date(DATE_ATOM)
	);

	redis()->rpush("u:$user_id:s",$session_id);

	redis()->hsetnx("u:$user_id","name",$profile_response['username']);
	$picture="https://cdn.discordapp.com/avatars/)$user_id}/{$profile_response['avatar']}.png";

	redis()->hsetnx("u:$user_id","locale",$profile_response['locale']);

	$session_id=bin2hex($session_id);

	header(HEADER_SESSION.": $session_id");

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

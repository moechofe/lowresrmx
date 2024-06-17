<?php

require_once __DIR__.'/common.php';

if($url['path']==='/twitter')
{
	// Sauce: https://developer.x.com/en/docs/authentication/oauth-2-0/application-only
	// Doc:
	// - https://developer.x.com/en/docs/authentication/api-reference/token

	$opt=array('http'=>array(
		'method'=>'POST',
		'header'=>join("\r\n",[
			'Authorization: Basic '.base64_encode(urlencode(TWITTER_API_KEY).':'.urlencode(TWITTER_API_KEY_SECRET)),
			'Content-Type: application/x-www-form-urlencoded;charset=UTF-8'
		]),
		'content'=>http_build_query(array(
			'grant_type'=>'client_credentials'))
	));

	$token=json_decode(
	file_get_contents("https://api.twitter.com/oauth2/token",false,stream_context_create($opt)),true);

	if(@$token['token_type']!=='bearer') return internalServerError("Fail to get token");

	var_dump($token);

	exit;
}

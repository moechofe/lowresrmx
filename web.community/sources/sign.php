<?php // API to check if the user is signed in and get some of their info.

require_once __DIR__.'/common.php';

if(preg_match('/^\/google$/',$urlPath)&&$isGet&&!empty($_GET['code'])&&!empty($_GET['state']))
{
	error_log(__FILE__.":".__LINE__);

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
	redis()->hsetnx("u:$user_id","locale",@$profile_response['locale']?:'en');

	$session_id=bin2hex($session_id);

	setcookie(
		HEADER_SESSION_COOKIE,$session_id,
		time()+SESSION_TTL,
		"/",
		"",
		isset($_SERVER['HTTPS']),
		true);

	// If a upload token is passed, the user is using the iOS app to share a program.
	if($uptoken) $location='/share?uptoken='.$uptoken;
	else $location='/community.html';
	error_log("Location: $location");

	header("Location: $location");
	exit;
}

elseif(preg_match('/^\/google$/',$urlPath)&&$isGet)
{
	error_log(__FILE__.":".__LINE__);
	// Sauce: https://developers.google.com/identity/protocols/oauth2/web-server

	$sequence=redis()->incr('seq:google');
	$state=sodium_crypto_shorthash(strval($sequence),LOGIN_GOOGLE_KEY);

	// If a upload token is passed, the user is using the iOS app to share a program.
	$uptoken=@$_GET['uptoken'];
	redis()->set("l:$state",$uptoken,"ex",LOGIN_GOOGLE_TTL);

	$config=json_decode(file_get_contents("https://accounts.google.com/.well-known/openid-configuration"),true);

	$auth_request=$config['authorization_endpoint'].'?'.http_build_query([
		'client_id'=>GOOGLE_CLIENT_ID,
		'redirect_uri'=>WEBSITE_URL.$urlPath,
		'response_type'=>'code',
		'scope'=>'openid profile',
		'access_type'=>'offline',
		'state'=>bin2hex($state),
	]);
	error_log("Auth request: $auth_request");

	header("Location: $auth_request");
	exit;
}

if(preg_match('/^\/discord$/',$urlPath)&&$isGet&&!empty($_GET['code'])&&!empty($_GET['state']))
{
	error_log(__FILE__.":".__LINE__);

	$state=hex2bin($_GET['state']);
	if(!redis()->exists("l:$state")) forbidden("Fail to validate state");

	// If a upload token is passed, the user is using the iOS app to share a program.
	$uptoken=redis()->get("l:$state");
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
	redis()->expire("s:$session_id",SESSION_TTL);

	redis()->rpush("u:$user_id:s",$session_id);

	redis()->hsetnx("u:$user_id","name",$profile_response['username']);
	$picture="https://cdn.discordapp.com/avatars/)$user_id}/{$profile_response['avatar']}.png";
	redis()->hset("u:$user_id","picture",$picture);
	redis()->hsetnx("u:$user_id","author",$profile_response['username']);
	redis()->hsetnx("u:$user_id","locale",$profile_response['locale']?:'en');

	$session_id=bin2hex($session_id);

	setcookie(
		HEADER_SESSION_COOKIE,$session_id,
		time()+SESSION_TTL,
		"/",
		"",
		isset($_SERVER['HTTPS']),
		true);

	// If a upload token is passed, the user is using the iOS app to share a program.
	if($uptoken) $location='/share?uptoken='.$uptoken;
	else $location='/community.html';
	error_log("Location: $location");

	header("Location: $location");
	exit;
}

elseif(preg_match('/^\/discord$/',$urlPath)&&$isGet)
{
	error_log(__FILE__.":".__LINE__);
	// Sauce: https://discord.com/developers/docs/topics/oauth2

	$sequence=redis()->incr('seq:discord');
	$state=sodium_crypto_shorthash(strval($sequence),LOGIN_DISCORD_KEY);

	// If a upload token is passed, the user is using the iOS app to share a program.
	$uptoken=@$_GET['uptoken'];
	redis()->set("l:$state",$uptoken,"ex",LOGIN_GOOGLE_TTL);

	$auth_request="https://discord.com/oauth2/authorize?".http_build_query([
		'client_id'=>DISCORD_CLIENT_ID,
		'response_type'=>'code',
		'redirect_uri'=>WEBSITE_URL.$urlPath,
		'scope'=>'identify',
		'state'=>bin2hex($state),
	]);
	error_log("Auth request: $auth_request");

	header("Location: $auth_request");
	exit;
}

if(preg_match('/^\/is_signed$/',$urlPath)&&$isPost)
{
	error_log(__FILE__);

	$user_id=validateSessionAndGetUserId();
	if(!$user_id)
	{
		header("Content-Type: application/json",true);
		echo json_encode(false);
		exit;
	}

	list($picture,$author)=redis()->hmget("u:$user_id","picture","author");

	header("Content-Type: application/json",true);
	echo json_encode(compact("picture","author"));

	exit;
}

if(preg_match('/^\/sign_out$/',$urlPath)&&$isPost)
{
	error_log(__FILE__);

	$session_id=@hex2bin(@$_COOKIE[HEADER_SESSION_COOKIE]);
	if($session_id) revokeSession($session_id);

	setcookie(
		HEADER_SESSION_COOKIE,"",
		time()-3600,
		"/",
		"",
		isset($_SERVER['HTTPS']),
		true);

	header("Content-Type: application/json",true);
	echo json_encode(true);

	exit;
}

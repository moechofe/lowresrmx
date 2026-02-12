<?php // API to check if the user is signed in and get some of their info.

require_once __DIR__.'/common.php';
require_once __DIR__.'/token.php';

if(preg_match('/^\/google$/',$urlPath)&&$isGet&&!empty($_GET['code'])&&!empty($_GET['state']))
{
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
			'redirect_uri'=>"{$baseUrl}{$urlPath}",
		])
	]];
	// error_log("Token request: ".json_encode($token_request));
	$token_response=json_decode(file_get_contents($config['token_endpoint'],false,stream_context_create($token_request)),true);
	// error_log("Token response: ".json_encode($token_response));
	if(empty($token_response['access_token'])||empty($token_response['token_type'])) internalServerError("Fail to read access token");

	$profile_request=['http'=>[
		'method'=>'GET',
		'header'=>"Authorization: {$token_response['token_type']} {$token_response['access_token']} \r\n"
	]];
	// error_log("Profile request: ".json_encode($profile_request));
	$profile_response=json_decode(file_get_contents("https://www.googleapis.com/userinfo/v2/me",false,stream_context_create($profile_request)),true);
	// error_log("Profile response: ".json_encode($profile_response));
	if(empty($profile_response['id'])) internalServerError("Fail to read google id");

	$user_id="{$profile_response['id']}.go2";

	$session_id=random_bytes(32); // stored in browser cookie
	$csrf_token=random_bytes(32); // stored in browser memory

	redis()->hmset("s:$session_id",
		"uid",$user_id,
		"status","allowed",
		"ct",date(DATE_ATOM),
		"csrf",$csrf_token,
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
		[
			'expires'=>time()+SESSION_TTL,
			'path'=>"/",
			'domain'=>'',
			'secure'=>$isHttps,
			'httponly'=>true,
			'samesite'=>$isHttps?'Strict':'Lax'
		]);

	// If a upload token is passed, the user is using the iOS app to share a program.
	if($uptoken) $location='/share?uptoken='.$uptoken;
	else $location='/community.html';
	header("Location: $location",true,302);
	exit;
}

elseif(preg_match('/^\/google$/',$urlPath)&&$isGet)
{
	if(!checkRateLimit('login',getClientIP())) tooManyRequests("Fail to respect limit");

	// Sauce: https://developers.google.com/identity/protocols/oauth2/web-server

	$sequence=redis()->incr('seq:google');
	$state=sodium_crypto_shorthash(strval($sequence),LOGIN_GOOGLE_KEY);

	// If a upload token is passed, the user is using the iOS app to share a program.
	if(@preg_match("/^($MATCH_ENTRY_TOKEN)$/",@$_GET['uptoken'],$matches))
		$uptoken=$matches[1];
	else
		$uptoken="";

	redis()->set("l:$state",$uptoken,"ex",LOGIN_GOOGLE_TTL);

	$config=json_decode(file_get_contents("https://accounts.google.com/.well-known/openid-configuration"),true);

	$auth_request=$config['authorization_endpoint'].'?'.http_build_query([
		'client_id'=>GOOGLE_CLIENT_ID,
		'redirect_uri'=>"{$baseUrl}{$urlPath}",
		'response_type'=>'code',
		'scope'=>'openid profile',
		'access_type'=>'offline',
		'state'=>bin2hex($state),
	]);
	header("Location: $auth_request",true,302);
	exit;
}

if(preg_match('/^\/discord$/',$urlPath)&&$isGet&&!empty($_GET['code'])&&!empty($_GET['state']))
{
	$state=hex2bin($_GET['state']);
	if(!redis()->exists("l:$state")) forbidden("Fail to validate state");

	// If a upload token is passed, the user is using the iOS app to share a program.
	$uptoken=redis()->get("l:$state");
	redis()->del("l:$state");

	// Sauce: https://discord.com/developers/docs/topics/oauth2#shared-resources-oauth2-urls

	$token_request=['http'=>[
		'method'=>'POST',
		'header'=>'Content-Type: application/x-www-form-urlencoded',
		'content'=>http_build_query([
			'client_id'=>DISCORD_CLIENT_ID,
			'client_secret'=>DISCORD_CLIENT_SECRET,
			'code'=>$_GET['code'],
			'grant_type'=>'authorization_code',
			'redirect_uri'=>"{$baseUrl}{$urlPath}",
		])
	]];
	// error_log("Token request: ".json_encode($token_request));
	$token_response=json_decode(file_get_contents("https://discord.com/api/oauth2/token",false,stream_context_create($token_request)),true);
	// error_log("Token Response: ".json_encode($token_response));
	if(empty($token_response['access_token'])) internalServerError("Fail to read access token");

	$profile_request=['http'=>[
		'method'=>'GET',
		'header'=>"Authorization: {$token_response['token_type']} {$token_response['access_token']}\r\n"
	]];
	// error_log("Profile request: ".json_encode($profile_request));
	$profile_response=json_decode(file_get_contents("https://discord.com/api/users/@me",false,stream_context_create($profile_request)),true);
	// error_log("Profile response: ".json_encode($profile_response));

	// TODO: add error here

	$user_id="{$profile_response['id']}.di2";

	$session_id=random_bytes(32); // stored in browser cookie
	$csrf_token=random_bytes(32); // stored in browser memory

	redis()->hmset("s:$session_id",
		"uid",$user_id,
		"status","allowed",
		"ct",date(DATE_ATOM),
		"csrf",$csrf_token,
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
		[
			'expires'=>time()+SESSION_TTL,
			'path'=>"/",
			'domain'=>'',
			'secure'=>$isHttps,
			'httponly'=>true,
			'samesite'=>$isHttps?'Strict':'Lax'
		]);

	// If a upload token is passed, the user is using the iOS app to share a program.
	if($uptoken) $location='/share?uptoken='.$uptoken;
	else $location='/community.html';
	header("Location: $location",true,302);
	exit;
}

elseif(preg_match('/^\/discord$/',$urlPath)&&$isGet)
{
	if(!checkRateLimit('login',getClientIP())) tooManyRequests("Fail to respect limit");

	// Sauce: https://discord.com/developers/docs/topics/oauth2

	$sequence=redis()->incr('seq:discord');
	$state=sodium_crypto_shorthash(strval($sequence),LOGIN_DISCORD_KEY);

	// If a upload token is passed, the user is using the iOS app to share a program.
	if(@preg_match("/^($MATCH_ENTRY_TOKEN)$/",@$_GET['uptoken'],$matches))
		$uptoken=$matches[1];
	else
		$uptoken="";

	redis()->set("l:$state",$uptoken,"ex",LOGIN_DISCORD_TTL);

	$auth_request="https://discord.com/oauth2/authorize?".http_build_query([
		'client_id'=>DISCORD_CLIENT_ID,
		'response_type'=>'code',
		'redirect_uri'=>"{$baseUrl}{$urlPath}",
		'scope'=>'identify',
		'state'=>bin2hex($state),
	]);
	// error_log("Auth request: $auth_request");

	header("Location: $auth_request");
	exit;
}

elseif(preg_match('/^\/github$/',$urlPath)&&$isGet&&!empty($_GET['code'])&&!empty($_GET['state']))
{
	error_log("State: ".json_encode($_GET['state']));
	$state=hex2bin($_GET['state']);
	if(!redis()->exists("l:$state")) forbidden("Fail to validate state");

	// If a upload token is passed, the user is using the iOS app to share a program.
	$uptoken=redis()->get("l:$state");
	redis()->del("l:$state");

	// Sauce: https://docs.github.com/en/apps/oauth-apps/building-oauth-apps/authorizing-oauth-apps#2-users-are-redirected-back-to-your-site-by-github

	$token_request=['http'=>[
		'method'=>'POST',
		'header'=>
			"Content-Type: application/x-www-form-urlencoded\r\n".
			"Accept: application/json\r\n".
			"User-Agent: Retro-Game-Creator\r\n".
			"X-GitHub-Api-Version: 2022-11-28",
		'content'=>http_build_query([
			'client_id'=>GITHUB_CLIENT_ID,
			'client_secret'=>GITHUB_CLIENT_SECRET,
			'code'=>$_GET['code'],
			'redirect_uri'=>"{$baseUrl}{$urlPath}",
		])
	]];
	error_log("Token request: ".json_encode($token_request));

	$token_response=json_decode(file_get_contents("https://github.com/login/oauth/access_token",false,stream_context_create($token_request)),true);
	error_log("Token response: ".json_encode($token_response));
	if(empty($token_response['access_token'])||empty($token_response['token_type'])) internalServerError("Fail to read access token");

	$profile_request=['http'=>[
		'method'=>'GET',
		'header'=>
			"Authorization: {$token_response['token_type']} {$token_response['access_token']}\r\n".
			"Accept: application/json\r\n".
			"User-Agent: Retro-Game-Creator\r\n".
			"X-GitHub-Api-Version: 2022-11-28",
		]
	];
	error_log("Profile request: ".json_encode($profile_request));

	$profile_response=json_decode(file_get_contents("https://api.github.com/user",false,stream_context_create($profile_request)),true);
	error_log("Profile response: ".json_encode($profile_response));
	if(empty($profile_response['id']) || empty($profile_response['login'])) internalServerError("Fail to read github id");

	$user_id="{$profile_response['id']}.gh2";

	$session_id=random_bytes(32); // stored in browser cookie
	$csrf_token=random_bytes(32); // stored in browser memory

	redis()->hmset("s:$session_id",
		"uid",$user_id,
		"status","allowed",
		"ct",date(DATE_ATOM),
		"csrf",$csrf_token,
	);
	redis()->expire("s:$session_id",SESSION_TTL);

	redis()->rpush("u:$user_id:s",$session_id);

	redis()->hsetnx("u:$user_id","name",$profile_response['login']);
	redis()->hset("u:$user_id","picture",@$profile_response['avatar_url']);
	redis()->hsetnx("u:$user_id","author",$profile_response['login']);
	// GitHub doesn't provide locale, so we can't set it.

	$session_id=bin2hex($session_id);

	setcookie(
		HEADER_SESSION_COOKIE,$session_id,
		[
			'expires'=>time()+SESSION_TTL,
			'path'=>"/",
			'domain'=>'',
			'secure'=>$isHttps,
			'httponly'=>true,
			'samesite'=>$isHttps?'Strict':'Lax'
		]);

	// If a upload token is passed, the user is using the iOS app to share a program.
	if($uptoken) $location='/share?uptoken='.$uptoken;
	else $location='/community.html';
	header("Location: $location",true,302);
	exit;
}

elseif(preg_match('/^\/github$/',$urlPath)&&$isGet)
{
	// if(!checkRateLimit('login',getClientIP())) tooManyRequests("Fail to respect limit");

	// Sauce: https://docs.github.com/en/apps/oauth-apps/building-oauth-apps/authorizing-oauth-apps#1-request-a-users-github-identity

	$sequence=redis()->incr('seq:github');
	$state=sodium_crypto_shorthash(strval($sequence),LOGIN_GITHUB_KEY);
	error_log("State: ".json_encode(bin2hex($state)));

	// If a upload token is passed, the user is using the iOS app to share a program.
	if(@preg_match("/^($MATCH_ENTRY_TOKEN)$/",@$_GET['uptoken'],$matches))
		$uptoken=$matches[1];
	else
		$uptoken="";

	redis()->set("l:$state",$uptoken,"ex",LOGIN_GITHUB_TTL);

	$auth_request="https://github.com/login/oauth/authorize?".http_build_query([
		'client_id'=>GITHUB_CLIENT_ID,
		'redirect_uri'=>"{$baseUrl}{$urlPath}",
		'state'=>bin2hex($state),
	]);
	error_log("Auth request: $auth_request");

	header("Location: $auth_request");
	exit;
}

elseif(preg_match('/^\/is_signed$/',$urlPath)&&$isPost)
{
	list($user_id,$token)=validateSessionAndGetUserId();
	if(!$user_id)
	{
		// header("Content-Type: application/json",true);
		// echo json_encode(false);

		setcookie(
			HEADER_SESSION_COOKIE,"",
			[
				'expires'=>time()-3600,
				'path'=>'/',
				'domain'=>'',
				'secure'=>$isHttps,
				'httponly'=>true,
				'samesite'=>$isHttps?'Strict':'Lax'
			]);

		header("Location: /community.html",true,302);
		exit;
	}

	list($picture,$author)=redis()->hmget("u:$user_id","picture","author");

	$token=bin2hex($token);
	header("Content-Type: application/json",true);
	header("X-Robots-Tag: noindex", true);
	echo json_encode(compact("picture","author","token"));

	exit;
}

elseif(preg_match('/^\/sign_out$/',$urlPath)&&$isPost)
{
	list($user_id,$csrf_token)=validateSessionAndGetUserId();
	// if(!validateCSRF(stored_token: $csrf_token)) forbidden("Fail to read token");

	$session_id=@hex2bin(@$_COOKIE[HEADER_SESSION_COOKIE]);
	if($session_id) revokeSession($session_id);

	setcookie(
		HEADER_SESSION_COOKIE,"",
		[
			'expires'=>time()-3600,
			'path'=>'/',
			'domain'=>'',
			'secure'=>$isHttps,
			'httponly'=>true,
			'samesite'=>$isHttps?'Strict':'Lax'
		]);

	header("Content-Type: application/json",true);
	header("X-Robots-Tag: noindex", true);
	echo json_encode(true);

	exit;
}

<?php

require_once __DIR__.'/common.php';

if($url['path']==='/discord')
{
	// Sauce: https://discord.com/developers/docs/topics/oauth2

	$sequence=redis()->incr('seq:discord');
	$discord_state=sodium_crypto_shorthash(strval($sequence),LOGIN_DISCORD_KEY);

	redis()->setex("l:$discord_state",LOGIN_DISCORD_TTL,$discord_state);

	$auth_request="https://discord.com/oauth2/authorize?".http_build_query([
		'client_id'=>DISCORD_CLIENT_ID,
		'response_type'=>'code',
		'redirect_uri'=>WEBSITE_URL.'/discord',
		'scope'=>'identify',
		'state'=>bin2hex($discord_state),
	]);

	header("Location: $auth_request");
	exit;
}


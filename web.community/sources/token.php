<?php

require_once __DIR__.'/common.php';

if($url['path']==='/token')
{
	$user_id=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");

	// TODO: count token per user and prevent re-creation

	$sequence=redis()->incr("seq:token");
	$hash=bin2hex(sodium_crypto_shorthash(strval($sequence),UPLOAD_TOKEN_KEY));

	$token="";
	for($i=0;$i<2;++$i) $token.=SYLLABLE_LIST[random_int(0,count(SYLLABLE_LIST))];
	$token.="-";
	for($i=0;$i<2;++$i) $token.=SYLLABLE_LIST[random_int(0,count(SYLLABLE_LIST))];
	$token.="-";
	$token.=$hash;

	// store temporary upload token, to validate the authorization of uploading a program by the user.
	redis()->hset("t:$token","uid",$user_id);
	redis()->expire("t:$token",UPLOAD_TOKEN_TTL);

	header(HEADER_TOKEN.": ".$token);
	exit;
}

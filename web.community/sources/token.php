<?php

require_once __DIR__.'/common.php';

function generateUploadToken():string
{
	$sequence=redis()->incr("seq:uptoken");
	$hash=bin2hex(sodium_crypto_shorthash(strval($sequence),UPLOAD_TOKEN_KEY));

	$token="";
	for($i=0;$i<2;++$i) $token.=SYLLABLE_LIST[random_int(0,count(SYLLABLE_LIST))];
	$token.="-";
	for($i=0;$i<2;++$i) $token.=SYLLABLE_LIST[random_int(0,count(SYLLABLE_LIST))];
	$token.="-";
	$token.=$hash;

	return $token;
}

function generateEntryToken():string
{
	$sequence=redis()->incr("seq:entry");
	$hash=bin2hex(sodium_crypto_shorthash(strval($sequence),ENTRY_TOKEN_KEY));

	$token="";
	for($i=0;$i<2;++$i) $token.=SYLLABLE_LIST[random_int(0,count(SYLLABLE_LIST))];
	$token.="-";
	for($i=0;$i<2;++$i) $token.=SYLLABLE_LIST[random_int(0,count(SYLLABLE_LIST))];
	$token.="-";
	$token.=$hash;

	return $token;
}

function validateEntryToken(string $token):string|null
{
	if(preg_match('/^[a-z]{3,}-[a-z]{3,}-[0-9a-f]{16}$/',$token)) return $token;
	return null;
}

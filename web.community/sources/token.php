<?php

require_once __DIR__.'/common.php';

function generateUploadToken():string
{
	$sequence=redis()->incr("seq:uptoken");
	$hash=bin2hex(sodium_crypto_shorthash(strval($sequence),UPLOAD_TOKEN_KEY));

	$token="'";
	for($i=0;$i<2;++$i) $token.=SYLLABLE_LIST[random_int(0,count(SYLLABLE_LIST))];
	$token.="-";
	for($i=0;$i<2;++$i) $token.=SYLLABLE_LIST[random_int(0,count(SYLLABLE_LIST))];
	$token.="-";
	$token.=$hash;

	return $token;
}

$MATCH_UPLOAD_TOKEN='[a-z\']{2,15}-[a-z\']{2,15}-[0-9a-fA-F]{16}';

function generateEntryToken():string
{
	$sequence=redis()->incr("seq:entry");
	$hash=bin2hex(sodium_crypto_shorthash(strval($sequence),ENTRY_TOKEN_KEY));

	$token="'";
	for($i=0;$i<2;++$i) $token.=SYLLABLE_LIST[random_int(0,count(SYLLABLE_LIST))];
	$token.="-";
	for($i=0;$i<2;++$i) $token.=SYLLABLE_LIST[random_int(0,count(SYLLABLE_LIST))];
	$token.="-";
	$token.=$hash;

	return $token;
}

$MATCH_ENTRY_TOKEN='[a-z\']{2,15}-[a-z\']{2,15}-[0-9a-fA-F]{16}';

// TODO: Is it useful?
function validateEntryToken(string $token):string|null
{
	global $MATCH_ENTRY_TOKEN;
	if(preg_match("/^$MATCH_ENTRY_TOKEN}\$/",$token)) return $token;
	return null;
}

<?php

require_once __DIR__.'/common.php';

if($url['path']==='/upload')
{
	$source=file_get_contents('php://input');
	if(empty($source)) badRequest("Fail to read input");

	$type=getallheaders()[HEADER_FILE_TYPE];
	if(!in_array($type,['prg','img'])) badRequest("Fail to read type header");

	$token=getallheaders()[HEADER_TOKEN];
	if(empty($token)) badRequest("Fail to read token header");

	$user_id=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");

	// check for the token
	$stored_user_id=redis()->hget("t:$token","uid");
	if($user_id!==$stored_user_id) forbidden("Fail to validate token");

	// store the uploaded file
	$source=zstd_compress($source);
	redis()->hset("t:$token",$type,$source);
	redis()->expire("t:$token",UPLOAD_TOKEN_TTL);

	exit;
}


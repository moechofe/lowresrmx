<?php

require_once __DIR__.'/common.php';

if($url['path']==='/publish')
{
	$token=getallheaders()[HEADER_TOKEN];
	if(empty($token)) badRequest("Fail to read token header");

	$user_id=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");

	// check for the token
	$stored_user_id=redis()->hget("t:$token","uid");
	if($user_id!==$stored_user_id) forbidden("Fail to validate token");

	// check for a program file
	if(!redis()->hget("t:$token","prg")) internalServerError("Fail to validate program");

	// TODO: check for an image file

	$program_id=$token;

	$author=redis()->hget("u:$user_id","name");

	redis()->persist("t:$token");
	redis()->rename("t:$token","p:$program_id");
	redis()->hset("p:$program_id","ct",date(DATE_ATOM));
	redis()->hset("p:$program_id","author",substr($author,0,MAX_AUTHOR_NAME));

	redis()->rpush("u:{$user_id}:p",$program_id);

	exit;
}

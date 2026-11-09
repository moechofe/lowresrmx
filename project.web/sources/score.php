<?php

require_once __DIR__.'/common.php';
require_once __DIR__.'/token.php';
require_once __DIR__.'/rank.php';

// API to upvote/downvote an entry
if(preg_match("/^\/($MATCH_ENTRY_TOKEN)\/vote$/",$urlPath,$matches)&&$isGet)
{
	list($user_id,$csrf_token)=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");
	if(!validateCSRF($csrf_token)) forbidden("Fail to read token");
	if(!checkRateLimit('vote',$user_id)) tooManyRequests("Fail to respect limit");

	$first_id=$matches[1];
	if(!$first_id) badRequest("Fail to read entry");
	if(!redis()->exists("f:$first_id:f")) badRequest("Fail to validate entry");
	list($post_uid,$status)=redis()->hmget("f:$first_id:f","uid","status");
	if($status==="banned") badRequest("Fail to validate entry");
	if($post_uid===$user_id) forbidden("Fail to vote own entry");

	$upvoted=intval(redis()->sadd("r:$first_id:v",$user_id))===1;
	if($upvoted)
	{
		track('vote');
		redis()->hset("r:$first_id:d","at",time());
		redis()->set("u:$user_id:t",time());
	}
	else redis()->srem("r:$first_id:v",$user_id);

	$points=updRank($first_id);

	header("Content-Type: application/json",true);
	header("X-Robots-Tag: noindex", true);
	echo json_encode([
		"upv"=>$upvoted,
		"pts"=>$points,
	]);
	exit;
}

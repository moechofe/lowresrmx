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

	$upvoted=redis()->sismember("r:$first_id:v",$user_id)?true:false;
	if(!$upvoted)
	{
		// add the vote
		redis()->sadd("r:$first_id:v",$user_id);
		$upvoted=true;
		redis()->hincrby("r:$first_id:d","vote",1);
	}
	else
	{
		// remove the vote
		redis()->srem("r:$first_id:v",$user_id);
		$upvoted=false;
		redis()->hincrby("r:$first_id:d","vote",-1);
	}

	$points=updRank($first_id);

	header("Content-Type: application/json",true);
	header("X-Robots-Tag: noindex", true);
	echo json_encode([
		"upv"=>$upvoted,
		"pts"=>$points,
	]);
	exit;
}

// if(preg_match("/^\/updrank$/",$urlPath)&&$isGet&&@getallheaders()[HEADER_ADMIN_ACCESS]===ADMIN_ACCESS_SECRET)
// {
// 	require_once __DIR__.'/updrank.php';

// 	header("Content-Type: application/json",true);
// 	echo json_encode(intval($new_cursor));
// 	exit;
// }

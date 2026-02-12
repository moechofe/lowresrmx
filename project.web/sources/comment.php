<?php

require_once __DIR__.'/common.php';
require_once __DIR__.'/token.php';
require_once __DIR__.'/rank.php';
require_once __DIR__.'/markdown.php';

// API to add a comment to a first entry post
if(preg_match("/\/($MATCH_ENTRY_TOKEN)\/comment$/",$urlPath,$matches)&&$isPost)
{
	$first_id=$matches[1];
	if(!$first_id) badRequest("Fail to read entry");

	list($user_id,$csrf_token)=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");
	if(!validateCSRF($csrf_token)) forbidden("Fail to read token");
	if(!checkRateLimit('comment',$user_id)) tooManyRequests("Fail to respect limit");

	$text=@file_get_contents('php://input');
	$text=mb_substr(@trim(@$text),0,MAX_POST_TEXT);
	if(empty($text)) badRequest("Fail to read text");

	// Check for the first post
	if(!redis()->exists("f:$first_id:f")) badRequest("Fail to validate program");
	$status=redis()->hget("f:$first_id:f","status");
	if($status==="banned") badRequest("Fail to validate entry");

	$author=redis()->hget("u:$user_id","name");
	$author=substr($author,0,MAX_AUTHOR_NAME);
	$text=zstd_compress($text);

	// Create the comment
	$cid=redis()->incr("f:$first_id:s");
	redis()->hmset("f:$first_id:$cid",
		"uid",$user_id,
		"text",$text,
		"ct",date(DATE_ATOM),
		"author",$author,
	);

	// Update date
	redis()->hset("f:$first_id:f",
		"ut",date(DATE_ATOM),
	);

	// Add to the user comment post list
	redis()->rpush("u:$user_id:c","$first_id:$cid");

	// Add the comment as child of the first post
	redis()->rpush("f:$first_id:c",$cid);

	// Give points to the user for the comment
	redis()->hincrby("r:$first_id:d","comm",1);

	updRank($first_id);

	header("Content-Type: application/json",true);
	header("X-Robots-Tag: noindex", true);
	echo json_encode(true);
	exit;
}

// API to retrieve comments of a first entry post.
if(preg_match("/\/($MATCH_ENTRY_TOKEN)\/(\d+)$/",$urlPath,$matches)&&$isGet)
{
	$first_id=$matches[1];
	if(!$first_id) badRequest("Fail to read entry");

	$skip=intval($matches[2]);
	if($skip<0) badRequest("Fail to read skip");

	// Build comments list
	$list=redis()->lrange("f:$first_id:c",$skip,10);
	$comments=[];
	foreach($list as $cid)
	{
		list($text,$ct,$author,$status)=redis()->hmget("f:$first_id:$cid","text","ct","author","status");
		if(empty($text)) continue;
		if($status==="banned") continue;
		$comments[]=[
			"text"=>markdown2html(zstd_uncompress($text)),
			"ct"=>$ct,
			"author"=>$author,
		];
	}

	header("Content-Type: application/json",true);
	header("X-Robots-Tag: noindex", true);
	echo json_encode($comments);
	exit;
}

<?php

require_once __DIR__.'/common.php';
require_once __DIR__.'/token.php';

// API to list the posts the user is involved in, flagging those with activity
// since the user last checked. Fetching this endpoint is itself the "seen"
// action: it advances the watermark to now.
if(preg_match('/^\/notif$/',$urlPath)&&$isGet)
{
	list($user_id,$csrf_token)=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");
	if(!validateCSRF($csrf_token)) forbidden("Fail to read token");
	if(!checkRateLimit('notif',$user_id)) tooManyRequests("Fail to respect limit");

	$now=time();
	// Read the watermark BEFORE advancing it
	$t=intval(redis()->get("u:$user_id:t"));

	// Newest involved posts first
	$ids=redis()->zrevrange("u:$user_id:n",0,49);
	$list=[];
	foreach($ids as $eid)
	{
		list($title,$author)=redis()->hmget("f:$eid:f","title","author");
		if(empty($title)) continue; // entry gone or invalid
		list($at,$comm,$vote)=redis()->hmget("r:$eid:d","at","comm","vote");
		$list[]=[
			"eid"=>$eid,
			"unseen"=>intval($at)>$t,
			"title"=>$title,
			"author"=>$author,
			"comm"=>intval($comm),
			"vote"=>intval($vote),
		];
	}

	// Advance the watermark: everything up to now is now considered seen
	redis()->set("u:$user_id:t",$now);

	header("Content-Type: application/json",true);
	header("X-Robots-Tag: noindex", true);
	echo json_encode($list);
	exit;
}

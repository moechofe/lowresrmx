<?php // Echo the HTML for a specific entry in the forum (a.k.a.: the first post of a thread).

require_once __DIR__.'/common.php';
require_once __DIR__.'/token.php';
require_once __DIR__.'/rank.php';

if(preg_match("/^\/($MATCH_ENTRY_TOKEN)\.html$/",$urlPath,$matches))
{
	$first_id=$matches[1];

	// Get the first entry post
	list($title,$text,$ut,$author,$status,$name)=redis()->hmget("f:$first_id:f","title","text","ut","author","status","name");
	if(empty($title) or empty($ut)) badRequest("Fail to read entry");
	if($status==="banned") badRequest("Fail to validate entry");
	if(!empty($text)) $text=markdown2html(zstd_uncompress($text));
	$points=redis()->hget("r:$first_id:d","pts");

	list($user_id,$csrf_token)=validateSessionAndGetUserId();
	$upvoted=false;
	if($user_id) $upvoted=redis()->sismember("r:$first_id:v",$user_id)==1?true:false;

	$eid=$first_id;
	require_once __DIR__.'/entry.html';
	exit;
}

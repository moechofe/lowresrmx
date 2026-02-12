<?php

require_once __DIR__.'/common.php';
require_once __DIR__.'/token.php';

if(preg_match("/\/($MATCH_ENTRY_TOKEN)\.player$/",$urlPath,$matches)&&$isGet)
{
	$eid=$matches[1];

	// Get the first entry post
	list($title,$author,$status)=redis()->hmget("f:$eid:f","title","author","status");
	if(empty($title) or empty($author)) badRequest("Fail to read entry");
	if($status==="banned") badRequest("Fail to validate entry");

	header("X-Robots-Tag: noindex", true);
	require_once __DIR__.'/player.html';
	exit;
}

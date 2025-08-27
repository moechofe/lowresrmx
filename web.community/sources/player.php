<?php

require_once __DIR__.'/common.php';
require_once __DIR__.'/token.php';

if(preg_match("/\/($MATCH_ENTRY_TOKEN)\.player$/",$urlPath,$matches))
{
	error_log(__FILE__);

	$eid=$matches[1];

	// Get the first entry post
	list($title,$ct,$author,$status)=redis()->hmget("f:$eid","title","ct","status");
	if(empty($title) or empty($ct)) badRequest("Fail to read entry");
	if($status==="banned") badRequest("Fail to validate entry");

	require_once __DIR__.'/player.html';
	exit;
}

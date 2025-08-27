<?php

require_once __DIR__.'/common.php';
require_once __DIR__.'/token.php';

if(preg_match("/^\/($MATCH_ENTRY_TOKEN)\.html$/",$urlPath,$matches))
{
	error_log(__FILE__);

	$eid=$matches[1];

	// Get the first entry post
	list($title,$text,$ct,$author,$status)=redis()->hmget("f:$eid:f","title","text","ct","author","status");
	if(empty($title) or empty($ct)) badRequest("Fail to read entry");
	if($status==="banned") badRequest("Fail to validate entry");
	if(!empty($text)) $text=zstd_uncompress($text);

	// // Get the comments
	// // TODO: pagination
	// $list=redis()->lrange("f:$eid:c",0,-9999);

	// $comments=[];

	// $l=count($list)-1;
	// for($i=$l;$i>=0;--$i)
	// {
	// 	list($text,$ct,$author,$status)=redis()->hget("f:$eid:$cid","text","ct","author","status");
	// 	$comments[]=[

	// 	];
	// }
	//

	$MAX_POST_TEXT=MAX_POST_TEXT;

	require_once __DIR__.'/entry.html';
	exit;
}

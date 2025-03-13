<?php

require_once __DIR__.'/common.php';

if(strlen($info['basename'])>16&&strlen($info['basename'])<512&&preg_match('/\b(\w+-\w+-[0-9a-fA-F]{16})\.html$/',$info['basename'],$matches))
{
	error_log(__FILE__);

	$eid=$matches[1];

	// Get the first entry post
	list($title,$text)=redis()->hmget("f:$eid","title","text");
	$text=zstd_uncompress($text);

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

	require_once __DIR__.'/entry.html';
	exit;
}

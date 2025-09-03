<?php

require_once __DIR__.'/common.php';
require_once __DIR__.'/token.php';

// API to increase play counter of an entry
if(preg_match("/^\/($MATCH_ENTRY_TOKEN)\/run$/",$urlPath,$matches)&&$isGet)
{
	error_log(__FILE__);

	$first_id=$$matches;
	if(!$first_id) badRequest("Fail to read entry");

	// TODO: continue

}

// if(preg_match("/^\/updrank$/",$urlPath)&&$isGet&&@getallheaders()[HEADER_ADMIN_ACCESS]===ADMIN_ACCESS_SECRET)
// {
// 	header("Content-Type: application/json",true);
// 	echo "{";
// 	$cursor=@intval(getallheaders()[HEADER_SCAN_CURSOR]);
// 	header(HEADER_SCAN_CURSOR.": $cursor");
// 	list($cursor,$list)=redis()->scan($cursor,"match","f:*:f","count",100);
// 	foreach($list as $item)
// 	{
// 		if(preg_match("/^f:($MATCH_ENTRY_TOKEN):f$/",$item,$matches))
// 		{
// 			$fid=$matches[1];
// 			list($ct,$upvote)=redis()->hmget("f:$fid:f","ct","upvote");
// 			echo $fid,$ct,$upvote;
// 		}
// 	}
// 	echo "}";
// }

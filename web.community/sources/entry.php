<?php // Echo the HTML for a specific entry in the forum (a.k.a.: the first post of a thread).

require_once __DIR__.'/common.php';
require_once __DIR__.'/token.php';
require_once __DIR__.'/rank.php';

if(preg_match("/^\/($MATCH_ENTRY_TOKEN)\.html$/",$urlPath,$matches))
{
	error_log(__FILE__);

	$first_id=$matches[1];

	// Get the first entry post
	list($title,$text,$ut,$author,$status)=redis()->hmget("f:$first_id:f","title","text","ut","author","status");
	if(empty($title) or empty($ut)) badRequest("Fail to read entry");
	if($status==="banned") badRequest("Fail to validate entry");
	if(!empty($text)) $text=zstd_uncompress($text);

	// Update the view counter
	redis()->hincrby("f:$first_id:r","view",1);

	updRank($first_id);

	$MAX_POST_TEXT=MAX_POST_TEXT;
	$eid=$first_id;

	require_once __DIR__.'/entry.html';
	exit;
}

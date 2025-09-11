<?php

require_once __DIR__.'/common.php';
require_once __DIR__.'/rank.php';

// API to post a topic to the forum.
if(preg_match('/\/post$/',$urlPath)&&$isPost)
{
	$user_id=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");

	// Check for field from the HTML form
	$json=json_decode(file_get_contents('php://input'),true);
	$title=mb_substr(@trim(@$json['i']),0,MAX_POST_TITLE);
	if(empty($title)) badRequest("Fail to read title");
	$text=mb_substr(@trim(@$json['x']),0,MAX_POST_TEXT);
	if(empty($text)) badRequest("Fail to read text");
	$where=@$json['w'];
	if(!in_array($where,TOPIC_VALID_FORUM)) badRequest("Fail to read where");

	// prepare text content
	$first_id=generateEntryToken();
	$author=redis()->hget("u:$user_id","name");
	$author=substr($author,0,MAX_AUTHOR_NAME);
	$text=zstd_compress($text);

	// Publish the post
	redis()->hmset("f:$first_id:f",
		"uid",$user_id,
		"title",$title,
		"text",$text,
		"ut",date(DATE_ATOM),
		"author",$author,
		"name",$name,
	);

	// Register the post in the forum
	redis()->zadd("w:$where",time(),$first_id);

	// Add to the user first post list
	redis()->lpush("u:{$user_id}:f",$first_id);

	// Give points to the user for the first post
	redis()->hmset("r:$first_id:d",
		"pts",POINTS_GIVEN['publish'],
		"vote",0,
		"comm",0,
		"w",$where,
		"ct",date(DATE_ATOM),
	);

	// Update the rank of the post
	updRank($first_id);

	header("Content-Type: application/json",true);
	echo json_encode($first_id);
	exit;
}

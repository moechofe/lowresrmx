<?php

require_once __DIR__.'/common.php';

if($url['path']==='/publish'&&$_SERVER['REQUEST_METHOD']==='POST')
{
	error_log(__FILE__);

	$user_id=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");

	// Check for field from the HTML form
	$json=json_decode(file_get_contents('php://input'),true);
	$program_id=@trim(@$json['p']);
	if(empty($program_id)) badRequest("Fail to read program");
	if(strlen($program_id)<=16||strlen($program_id)>=512) badRequest("Fail to read program");
	$title=mb_substr(@trim(@$json['i']),0,MAX_POST_TITLE);
	if(empty($title)) badRequest("Fail to read title");
	$text=mb_substr(@trim(@$json['x']),0,MAX_POST_TEXT);
	if(empty($text)) badRequest("Fail to read text");
	$where=@$json['w'];
	if(!in_array($where,FORUM_WHERE)) badRequest("Fail to read where");

	// Check for the program
	if(!redis()->exists("p:$program_id")) badRequest("Fail to validate program");
	list($prg,$img)=redis()->hmget("p:$program_id","prg","img");
	if(empty($prg)) internalServerError("Fail to read program");
	if(empty($img)) internalServerError("Fail to read image");

	$first_id=generateEntryToken();
	$author=redis()->hget("u:$user_id","name");
	$author=substr($author,0,MAX_AUTHOR_NAME);
	$text=zstd_compress($text);

	$folder=substr($first_id,0,3);
	mkdir(CONTENT_FOLDER.$folder,0777,true);
	file_put_contents(CONTENT_FOLDER."$folder/$first_id.rmx",$prg);
	file_put_contents(CONTENT_FOLDER."$folder/$first_id.png",$img);

	// Publish the program
	redis()->hmset("f:$first_id",
		"uid",$user_id,
		"title",$title,
		"text",$text,
		"ct",date(DATE_ATOM),
		"author",$author,
	);
	// Add to the user first post list
	redis()->lpush("u:{$user_id}:f",$first_id);
	// Register the post in the forum
	redis()->zadd("w:$where",SCORE_FOR_FIRST_POST,$first_id);
	// Mark the program as published (for the owner)
	redis()->hset("p:$program_id","first",$first_id);

	header("Content-Type: application/json",true);
	echo json_encode($first_id);
	exit;
}

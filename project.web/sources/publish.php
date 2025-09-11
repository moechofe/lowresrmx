<?php

require_once __DIR__.'/common.php';
require_once __DIR__.'/rank.php';

// API to publish a program to the forum.
if(preg_match('/\/publish$/',$urlPath)&&$isPost)
{
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
	if(!in_array($where,PROGRAM_VALID_FORUM)) badRequest("Fail to read where");

	// Check for the program
	if(!redis()->exists("p:$program_id")) badRequest("Fail to validate program");
	list($prg,$img,$name)=redis()->hmget("p:$program_id","prg","img","name");
	if(empty($prg)) internalServerError("Fail to read program");
	if(empty($img)) internalServerError("Fail to read image");
	if(empty($name)) internalServerError("Fail to read program name");

	// prepare text content
	$first_id=generateEntryToken();
	$author=redis()->hget("u:$user_id","name");
	$author=substr($author,0,MAX_AUTHOR_NAME);
	$text=zstd_compress($text);

	// stored private program from redis to public program to file
	$folder=substr($first_id,0,3);
	@mkdir(CONTENT_FOLDER.$folder,0777,true);
	$prg=zstd_uncompress($prg);
	if(empty($prg)) internalServerError("Fail to read program");
	if(!file_put_contents(CONTENT_FOLDER."$folder/$first_id.rmx",$prg)) internalServerError("Fail to write program file");
	if(!file_put_contents(CONTENT_FOLDER."$folder/$first_id.png",$img)) internalServerError("Fail to write image file");

	// Publish the program
	redis()->hmset("f:$first_id:f",
		"uid",$user_id,
		"title",$title,
		"text",$text,
		"ut",date(DATE_ATOM),
		"author",$author,
		"name",$name,
	);

	// Register the program in the forum
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

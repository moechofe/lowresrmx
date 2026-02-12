<?php

require_once __DIR__.'/common.php';
require_once __DIR__.'/admin.php';
require_once __DIR__.'/token.php';
require_once __DIR__.'/rank.php';
require_once __DIR__.'/markdown.php';

// Receive the upload token from /upload.php and store a program in Redis linked to the user who uploaded it.
// Then it will redirect to /share.html
// Allow the user to share a program from the iOS app. After the upload, show the share page or the sign-in page if the user is not connected. It will retrieve the temporary program from Redis and store it as files.
// Reached when user share a program from the iOS app, right after the upload.
// See: upload.php
if(preg_match('/^\/share$/',$urlPath)&&$isGet)
{
	$uptoken=@$_GET['uptoken'];
	if(empty($uptoken)) badRequest("Fail to read uptoken");

	list($user_id,$csrf_token)=validateSessionAndGetUserId();

	// This is necessary to redirect user to the login page
	if(!$user_id)
	{
		header("X-Robots-Tag: noindex", true);
		require __DIR__.'/sign-in.html';
		exit;
	}

	// I cant't check this because the crsf token whas not sent yet. I will later through /is-signed API request made by the browser.
	// if(!validateCSRF($csrf_token)) forbidden("Fail to read token");
	if(!checkRateLimit('share',$user_id)) tooManyRequests("Fail to respect limit");

	// Transfer the temporary program to a persistent one
	list($prg,$img,$name)=redis()->hmget("t:$uptoken","prg","img","name");
	if(empty($prg) || empty($img) || empty($name)) badRequest("Fail to read uploaded");

	$program_id=$uptoken;
	$author=redis()->hget("u:$user_id","author");

	// Store a persistent program
	redis()->hmset("p:$program_id",
		"uid",$user_id,
		"prg",$prg,
		"img",$img,
		"name",$name,
		"ct",date(DATE_ATOM),
		"author",$author,
	);

	// Add to the program list
	redis()->rpush("u:$user_id:p",$program_id);

	// Clean up the temporary program
	redis()->del("t:$uptoken");

	header("Location: /share.html");
	exit;
}

// API that return a list of last uploaded programs for the connected user as a JSON array.
if(preg_match('/^\/last_shared$/',$urlPath)&&$isGet)
{
	list($user_id,$csrf_token)=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");

	// TODO: handle more than 9999 programs.
	$programs=redis()->lrange("u:{$user_id}:p",-9999,-1);

	$list=[];

	// traverse the list of published programs in reverse order and gather information on each program.
	$l=count($programs)-1;
	for($i=$l;$i>=0;--$i)
	{
		list($name,$author,$ct,$first_id)=$prg=redis()->hmget("p:{$programs[$i]}","name","author","ct","first");
		if(empty($name)||empty($author))
		{
			cleanInvalidUserProgram($user_id,$programs[$i]);
			continue;
		}

		$entry=[
			'pid'=>$programs[$i],
			'name'=>$name,
			'author'=>$author,
			'ct'=>$ct,
		];

		if(!empty($first_id))
		{
			$points=intval(redis()->hget("r:$first_id:d","pts"));
			if(!empty($points))
			{
				$entry['eid']=$first_id;
				$entry['pts']=$points;
			}
		}

		$list[]=$entry;
	}

	header("Content-Type: application/json",true);
	echo json_encode($list);
	exit;
}

if(preg_match('/^\/delete$/',$urlPath)&&$isPost)
{
	list($user_id,$csrf_token)=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");
	if(!validateCSRF($csrf_token)) forbidden("Fail to read token");

	$json=json_decode(file_get_contents('php://input'),true);

	{
		$program_id=@trim(@$json['p']);
		if(empty($program_id)) badRequest("Fail to read program");
		if(strlen($program_id)<=16||strlen($program_id)>=512) badRequest("Fail to read program");

		redis()->del("p:$program_id");
		redis()->lrem("u:{$user_id}:p",1,$program_id);
	}

	exit;
}

// API to publish a program to the forum.
if(preg_match('/\/publish$/',$urlPath)&&$isPost)
{
	list($user_id,$csrf_token)=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");
	if(!validateCSRF($csrf_token)) forbidden("Fail to read token");
	if(!checkRateLimit('publish',$user_id)) tooManyRequests("Fail to respect limit");

	// Check for field from the HTML form
	$json=json_decode(file_get_contents(filename: 'php://input'),true);
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

	// Mark the program as publish
	redis()->hset("p:$program_id","first",$first_id);

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
	header("X-Robots-Tag: noindex", true);
	echo json_encode($first_id);
	exit;
}

// API to post a topic to the forum.
if(preg_match('/\/post$/',$urlPath)&&$isPost)
{
	list($user_id,$csrf_token)=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");
	if(!validateCSRF(stored_token: $csrf_token)) forbidden("Fail to read token");
	if(!checkRateLimit('post',$user_id)) tooManyRequests("Fail to respect limit");

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
	header("X-Robots-Tag: noindex", true);
	echo json_encode($first_id);
	exit;
}

// API that return a list of post published by a user
if(preg_match('/\/published$/',$urlPath)&&$isGet)
{
	if(!checkRateLimit('search',getClientIP())) tooManyRequests("Fail to respect limit");

	list($user_id,$csrf_token)=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");
	if(!validateCSRF($csrf_token)) forbidden("Fail to read token");

	if(@preg_match("/^($MATCH_ENTRY_TOKEN)$/",@rawurldecode(@$_GET['id']),$matches))
	{
		$first_id=$matches[1];

		$text=redis()->hget("f:$first_id:f","text");
		if(empty($text)) badRequest("Fail to read text");
		$text=zstd_uncompress($text);

		header("Content-Type: application/json",true);
		header("X-Robots-Tag: noindex", true);
		echo json_encode($text);
		exit;
	}
	else
	{
		$start=@intval(@getallheaders()[HEADER_SCAN_CURSOR],10);

		$programs=redis()->lrange("u:{$user_id}:f",$start,$start+9);
		$published=[];
		foreach($programs as $first_id)
		{
			list($title,$name)=redis()->hmget("f:$first_id:f","title","name");
			if(empty($title)||empty($name)) continue;
			$published[]=[
				'pid'=>$first_id,
				'title'=>$title,
				'name'=>$name,
			];
		}
		header("Content-Type: application/json",true);
		header("X-Robots-Tag: noindex", true);
		header(HEADER_SCAN_CURSOR.":".($start+9),true);
		echo json_encode($published);
		exit;
	}
}

if(preg_match("/\/($MATCH_ENTRY_TOKEN)\/replace$/",$urlPath,$matches)&&$isPost)
{
	$first_id=$matches[1];
	if(!$first_id) badRequest("Fail to read entry");
	error_log("first_id: $first_id");

	list($user_id,$csrf_token)=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");
	if(!validateCSRF($csrf_token)) forbidden("Fail to read token");
	if(!checkRateLimit('publish',$user_id)) tooManyRequests("Fail to respect limit");

	// Check for field from the HTML form
	$json=json_decode(file_get_contents(filename: 'php://input'),true);
	if(!preg_match("/^($MATCH_ENTRY_TOKEN)$/",$json['p'],$matches)) badRequest("Fail to read program");
	$program_id=$matches[0];
	$title=mb_substr(@trim(@$json['i']),0,MAX_POST_TITLE);
	if(empty($title)) badRequest("Fail to read title");
	$text=mb_substr(@trim(@$json['x']),0,MAX_POST_TEXT);
	if(empty($text)) badRequest("Fail to read text");
	error_log("program_id: $program_id");

	// Check for the program
	if(!redis()->exists("p:$program_id")) badRequest("Fail to validate program");
	list($prg,$img,$name)=redis()->hmget("p:$program_id","prg","img","name");
	if(empty($prg)) internalServerError("Fail to read program");
	if(empty($img)) internalServerError("Fail to read image");
	if(empty($name)) internalServerError("Fail to read program name");

	// Prepare text content
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
		"title",$title,
		"text",$text,
		"ut",date(DATE_ATOM),
		"author",$author,
		"name",$name,
	);

	// Mark the program as publish
	redis()->hset("p:$program_id","first",$first_id);

	header("Content-Type: application/json",true);
	exit;
}

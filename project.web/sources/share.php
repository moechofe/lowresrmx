<?php

require_once __DIR__.'/common.php';
require_once __DIR__.'/admin.php';

// Receive the upload token from /upload.php and store a program in Redis linked to the user who uploaded it.
// Then it will redirect to /share.html
// Allow the user to share a program from the iOS app. After the upload, show the share page or the sign-in page if the user is not connected. It will retrieve the temporary program from Redis and store it as files.
// Reached when user share a program from the iOS app, right after the upload.
// See: upload.php
if(preg_match('/^\/share$/',$urlPath)&&$isGet)
{
	$uptoken=@$_GET['uptoken'];
	if(empty($uptoken)) badRequest("Fail to read uptoken");

	$user_id=validateSessionAndGetUserId();

	// This is necessary to redirect user to the login page
	if(!$user_id)
	{
		require __DIR__.'/sign-in.html';
		exit;
	}

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
	$user_id=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");

	header("Content-Type: application/json",true);

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

	echo json_encode($list);
	exit;
}

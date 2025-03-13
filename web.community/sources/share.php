<?php // Receive the upload token from /upload.php and store a program in Redis linked to the user who uploaded it.
// Then it will redirect to /share.html
// Allow the user to share a program from the iOS app. After the upload, show the share page or the sign-in page if the user is not connected. It will retrieve the temporary program from Redis and store it as files.

require_once __DIR__.'/common.php';

// Reached when user share a program from the iOS app, right after the upload.
// See: upload.php
if($url['path']==='/share')
{
	error_log(__FILE__);

	$uptoken=@$_GET['uptoken'];
	if(empty($uptoken)) badRequest("Fail to read uptoken");

	$user_id=validateSessionAndGetUserId();
	error_log("User id: $user_id");

	if(!$user_id)
	{
		require __DIR__.'/sign-in.html';
		exit;
	}
	else
	{
		// Transfer the temporary program to a persistent one
		list($prg,$img,$name)=redis()->hmget("t:$uptoken","prg","img","name");
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
}

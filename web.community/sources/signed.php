<?php

require_once __DIR__.'/common.php';

if($url['path']==='/is_signed')
{
	$user_id=validateSessionAndGetUserId();
	if(!$user_id)
	{
		header("Content-Type: application/json",true);
		echo json_encode(false);
		exit;
	}

	list($picture,$author)=redis()->hmget("u:$user_id","picture","author");

	header("Content-Type: application/json",true);
	echo json_encode(compact("picture","author"));

	exit;
}

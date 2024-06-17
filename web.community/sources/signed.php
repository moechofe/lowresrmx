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

	list($name,$picture,$author)=redis()->hget("u:$user_id","name","picture","author");

	var_dump($name,$picture,$author);

	exit;
}

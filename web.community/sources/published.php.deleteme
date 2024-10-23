<?php

require_once __DIR__.'/common.php';

if($url['path']==='/published')
{
	$user_id=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");

	$published_list=redis()->lrange("u:{$user_id}:p",0,9999);

	header("Content-Type: application/json",true);
	echo json_encode($published_list);
	exit;
}

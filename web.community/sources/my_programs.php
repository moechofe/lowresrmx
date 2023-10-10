<?php

require_once __DIR__.'/common.php';

if($url['path']==='/my_programs')
{
	$user_id=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");

	$program_list=redis()->lrange("u:{$user_id}:p",0,25);
	$program_list=array_map(function($program_id){

		// redis()->hmget("p:$program_id","")

		return $program_id;

	},$program_list);

	header("Content-Type: application/json",true);
	echo json_encode($program_list);
	exit;
}

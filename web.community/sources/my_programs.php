<?php

require_once __DIR__.'/common.php';

if($url['path']==='/my_programs')
{
	$user_id=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");

	$len=redis()->llen("u:{$user_id}:p");
	$count=25;

	$start=max(0,$len-$count);
	$stop=$start+$count-1;

	$program_list=redis()->lrange("u:{$user_id}:p",$start,$stop);
	$program_list=array_map(function($program_id){
		list($name,$ct)=redis()->hmget("p:$program_id","name","ct");
		return [$program_id,$name,$ct];
	},$program_list);

	header("Content-Type: application/json",true);
	echo json_encode($program_list);
	exit;
}

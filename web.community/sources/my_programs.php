<?php // TODO: seems not used

require_once __DIR__.'/common.php';

if($url['path']==='/my_programs')
{
	$user_id=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");

	$len=redis()->llen("u:{$user_id}:p");
	$count=25;

	$start=max(0,$len-$count);
	$stop=$start+$count-1;

	$last_shared=redis()->lrange("u:{$user_id}:p",$start,$stop);
	$last_shared=array_map(function($program_id){
		list($name,$ct)=redis()->hmget("p:$program_id","name","ct");
		return [$program_id,$name,$ct];
	},$last_shared);

	header("Content-Type: application/json",true);
	echo json_encode($last_shared);
	exit;
}

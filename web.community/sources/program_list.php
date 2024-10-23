<?php // Return a list of last uploaded programs for the connected user as a JSON array.

require_once __DIR__.'/common.php';

if($url['path']==='/my_last_prg')
{
	error_log(__FILE__);

	$user_id=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");

	// TODO: handle more than 9999 programs.
	$published_list=redis()->lrange("u:{$user_id}:p",-9999,-1);

	$program_list=[];

	// traverse the list of published programs in reverse order and gather information on each program.
	$l=count($published_list)-1;
	for($i=$l;$i>=0;--$i)
	{
		list($name,$author,$ct)=$prg=redis()->hmget("p:{$published_list[$i]}","name","author","ct");
		$program_list[]=[
			'pid'=>$published_list[$i],
			'name'=>$name,
			'author'=>$author,
			'ct'=>$ct,
		];
	}

	header("Content-Type: application/json",true);
	echo json_encode($program_list);
	exit;
}

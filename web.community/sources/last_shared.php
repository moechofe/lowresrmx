<?php // API that return a list of last uploaded programs for the connected user as a JSON array.

require_once __DIR__.'/common.php';

if($url['path']==='/last_shared')
{
	error_log(__FILE__);

	$user_id=validateSessionAndGetUserId();
	if(!$user_id)
	{
		require __DIR__.'/sign-in.html';
		exit;
	}// forbidden("Fail to read user");

	header("Content-Type: application/json",true);

	// TODO: handle more than 9999 programs.
	$list=redis()->lrange("u:{$user_id}:p",-9999,-1);

	$last_shared=[];

	// traverse the list of published programs in reverse order and gather information on each program.
	$l=count($list)-1;
	for($i=$l;$i>=0;--$i)
	{
		list($name,$author,$ct)=$prg=redis()->hmget("p:{$list[$i]}","name","author","ct");
		$last_shared[]=[
			'pid'=>$list[$i],
			'name'=>$name,
			'author'=>$author,
			'ct'=>$ct,
		];
	}

	echo json_encode($last_shared);
	exit;
}

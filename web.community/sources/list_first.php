<?php // API that return a list of published programs (a.k.a.: the first entry of a thread) order by score.

require_once __DIR__.'/common.php';

if($url['path']==='/list_first')
{
	header("Content-Type: application/json",true);
	error_log(__FILE__);

	// TODO: handle more than 99 programs.7
	$list=redis()->zrange("w:show",-99,-1);

	$last_published=[];

	// traverse the list of published programs in reverse order and gather information on each program.
	$l=count($list)-1;
	for($i=$l;$i>=0;--$i)
	{
		list($title,$author,$ut,$upvote)=$prg=redis()->hmget("f:{$list[$i]}:f","title","author","ut");
		$last_published[]=[
			'pid'=>$list[$i],
			'title'=>$title,
			'author'=>$author,
			'ut'=>$ut,
		];
	}

	echo json_encode($last_published);
	exit;
}

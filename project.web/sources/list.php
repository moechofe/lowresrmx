<?php

require_once __DIR__.'/common.php';

// API that return a list of ranked programs as a JSON array.
if(preg_match('/\/ranked$/',$urlPath)&&$isGet)
{
	header("Content-Type: application/json",true);
	$where=@$_GET['w'];
	if(!in_array($where,PROGRAM_VALID_FORUM)) $where='all';

	// TODO: handle more than 99 programs.
	$list=redis()->zrevrange("r:$where",0,99);

	$published=[];

	// traverse the list of published programs in reverse order and gather information on each program.
	$l=count($list)-1;
	for($i=$l;$i>=0;--$i)
	{
		list($title,$author,$ut,$upvote,$name)=$prg=redis()->hmget("f:{$list[$i]}:f","title","author","ut","name");
		$points=redis()->hget("r:{$list[$i]}:d","pts");
		if(!empty($title)&&!empty($author)&&!empty($ut)) $published[]=[
			'pid'=>$list[$i],
			'title'=>$title,
			'author'=>$author,
			'points'=>$points,
			'ut'=>$ut,
			'name'=>$name,
		];
	}

	echo json_encode($published);
	exit;
}

if(preg_match('/\/latest$/',$urlPath)&&$isGet)
{
	header("Content-Type: application/json",true);
	$where=@$_GET['w'];
	if(!in_array($where,PROGRAM_VALID_FORUM)) $where='all';

	// TODO: handle more than 99 programs.
	$list=redis()->zrevrange("w:$where",0,99);

	$published=[];

	// traverse the list of published programs in reverse order and gather information on each program.
	$l=count($list)-1;
	for($i=$l;$i>=0;--$i)
	{
		list($title,$author,$ut,$upvote,$name)=$prg=redis()->hmget("f:{$list[$i]}:f","title","author","ut","name");
		$points=redis()->hget("r:{$list[$i]}:d","pts");
		if(!empty($title)&&!empty($author)&&!empty($ut)) $published[]=[
			'pid'=>$list[$i],
			'title'=>$title,
			'author'=>$author,
			'points'=>$points,
			'ut'=>$ut,
			'name'=>$name,
		];
	}

	echo json_encode($published);
	exit;
}

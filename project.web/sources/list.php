<?php

require_once __DIR__.'/common.php';
require_once __DIR__.'/admin.php';

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
		list($title,$author,$ut,$name)=$prg=redis()->hmget("f:{$list[$i]}:f","title","author","ut","name");
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

// API that return a list of programs sorted by the last one first.
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
		list($title,$author,$ut,$name)=$prg=redis()->hmget("f:{$list[$i]}:f","title","author","ut","name");
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

if(preg_match('/\/match$/',$urlPath)&&$isGet&&!empty($_GET['q']))
{
	$where=@$GET['w'];
	if(!in_array($where,PROGRAM_VALID_FORUM)) $where='all';

	$query=mb_substr(trim(urldecode($_GET['q'])),0,MAX_QUERY);
	if(empty($query)) badRequest("Fail to read query");
	$pattern='/'.addslashes($query).'/i';

	$cursor=max(0,intval(value: @getallheaders()[HEADER_SCAN_CURSOR]));

	list($new_cursor,$list)=redis()->zscan("r:all",$cursor);

	$matched=[$new_cursor];

	for($i=0;$i<count($list);$i=$i+2)
	{
		$first_id=$list[$i];

		list($title,$author,$ut,$text)=$prg=redis()->hmget("f:$first_id:f","title","author","ut","text");
		// invalid entry, clean it
		if(empty($title)||empty($author)||empty($ut)||empty($text)) { cleanInvalidFirst($first_id); continue; }
		if(!empty($text)) $text=zstd_uncompress($text);

		if(preg_match($pattern,$title)||preg_match($pattern,$text))
		{
			$points=redis()->hget("r:{$first_id}:d","pts");
			if(!empty($points)) $matched[]=[
				'pid'=>$first_id,
				'title'=>$title,
				'author'=>$author,
				'points'=>$points,
				'ut'=>$ut
			];
		}
	}

	header("Content-Type: application/json",true);
	echo json_encode($matched);
}

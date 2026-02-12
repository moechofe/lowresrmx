<?php

require_once __DIR__.'/common.php';
require_once __DIR__.'/admin.php';
require_once __DIR__.'/rank.php';
require_once __DIR__.'/markdown.php';

// API that return a list of ranked programs as a JSON array.
if(preg_match('/\/ranked$/',$urlPath)&&$isGet)
{
	$where=@$_GET['w'];
	if(!in_array($where,PROGRAM_VALID_FORUM)) $where='all';

	// TODO: handle more than 99 programs.
	$list=redis()->zrevrange("r:$where",0,99);

	$published=[];

	// traverse the list of published programs and gather information on each program.
	for($i=0;$i<count($list);++$i)
	{
		$first_id=$list[$i];

		list($title,$author,$ut,$name)=$prg=redis()->hmget("f:$first_id:f","title","author","ut","name");
		if(empty($title)||empty($author)||empty($ut)) { cleanInvalidFirst($first_id); continue; }
		$points=redis()->hget("r:$first_id:d","pts");
		$comm=redis()->hget("r:$first_id:d","comm");
		$published[]=[
			'pid'=>$list[$i],
			'title'=>$title,
			'author'=>$author,
			'points'=>$points,
			'comm'=>$comm,
			'ut'=>$ut,
			'name'=>$name,
		];
		}

	header("Content-Type: application/json",true);
	header("X-Robots-Tag: noindex", true);
	echo json_encode($published);
	exit;
}

// API that return a list of programs sorted by the last one first.
if(preg_match('/\/latest$/',$urlPath)&&$isGet)
{
	$where=@$_GET['w'];
	if(!in_array($where,PROGRAM_VALID_FORUM)) $where='all';

	// TODO: handle more than 99 programs.
	$list=redis()->zrevrange("w:$where",0,99);

	$published=[];

	// traverse the list of published programs and gather information on each program.
	for($i=0;$i<count($list);++$i)
	{
		$first_id=$list[$i];

		list($title,$author,$ut,$name)=$prg=redis()->hmget("f:$first_id:f","title","author","ut","name");
		if(empty($title)||empty($author)||empty($ut)) { cleanInvalidFirst($first_id); continue; }
		$points=redis()->hget("r:$first_id:d","pts");
		$comm=redis()->hget("r:$first_id:d","comm");
		$published[]=[
			'pid'=>$list[$i],
			'title'=>$title,
			'author'=>$author,
			'points'=>$points,
			'comm'=>$comm,
			'ut'=>$ut,
			'name'=>$name,
		];
	}

	header("Content-Type: application/json",true);
	header("X-Robots-Tag: noindex", true);
	echo json_encode($published);
	exit;
}

// API that return a list of ranked programs as a JSON array that match a query.
if(preg_match('/\/match$/',$urlPath)&&$isGet&&!empty($_GET['q']))
{
	if(!checkRateLimit('search',getClientIP())) tooManyRequests("Fail to respect limit");

	$where=@$_GET['w'];
	if(!in_array($where,PROGRAM_VALID_FORUM)) $where='all';

	$query=mb_substr(trim(urldecode($_GET['q'])),0,MAX_QUERY);
	$query=preg_replace('/[^\pL\pN ]+/','',$query);
	if(empty($query)) badRequest("Fail to read query");
	$pattern='/'.preg_quote($query).'/i';

	$cursor=max(0,intval(value: @getallheaders()[HEADER_SCAN_CURSOR]));

	list($cursor,$list)=redis()->zscan("r:all",$cursor);

	$matched=[];
	for($i=0;$i<count($list);$i=$i+2)
	{
		$first_id=$list[$i];

		list($title,$author,$ut,$text)=$prg=redis()->hmget("f:$first_id:f","title","author","ut","text");
		// invalid entry, clean it
		if(empty($title)||empty($author)||empty($ut)||empty($text)) { cleanInvalidFirst($first_id); continue; }
		if(!empty($text)) $text=markdown2html(zstd_uncompress($text));

		if(preg_match($pattern,$title)||preg_match($pattern,$text))
		{
			$points=redis()->hget("r:{$first_id}:d","pts");
			$comm=redis()->hget("r:$first_id:d","comm");
			if(!empty($points)) $matched[]=[
				'pid'=>$first_id,
				'title'=>$title,
				'author'=>$author,
				'points'=>$points,
				'comm'=>$comm,
				'ut'=>$ut,
			];
		}
	}

	header("Content-Type: application/json",true);
	header("X-Robots-Tag: noindex", true);
	header(HEADER_SCAN_CURSOR.":".$cursor,true);
	echo json_encode($matched);
}

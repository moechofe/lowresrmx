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
		if(!empty($text)) $text=zstd_uncompress($text);

		if(preg_match($pattern,$title)||preg_match($pattern,$text))
		{
			$points=redis()->hget("r:{$first_id}:d","pts");
			if(!empty($points)) $matched[]=[
				'pid'=>$first_id,
				'title'=>$title,
				'author'=>$author,
				'points'=>$points,
				'ut'=>$ut,
			];
		}
	}

	header("Content-Type: application/json",true);
	header(HEADER_SCAN_CURSOR.":".$cursor,true);
	echo json_encode($matched);
}

// API to post a topic to the forum.
if(preg_match('/\/post$/',$urlPath)&&$isPost)
{
	list($user_id,$csrf_token)=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");
	if(!validateCSRF(stored_token: $csrf_token)) forbidden("Fail to read token");
	if(!checkRateLimit('post',$user_id)) tooManyRequests("Fail to respect limit");

	// Check for field from the HTML form
	$json=json_decode(file_get_contents('php://input'),true);
	$title=mb_substr(@trim(@$json['i']),0,MAX_POST_TITLE);
	if(empty($title)) badRequest("Fail to read title");
	$text=mb_substr(@trim(@$json['x']),0,MAX_POST_TEXT);
	if(empty($text)) badRequest("Fail to read text");
	$where=@$json['w'];
	if(!in_array($where,TOPIC_VALID_FORUM)) badRequest("Fail to read where");

	// prepare text content
	$first_id=generateEntryToken();
	$author=redis()->hget("u:$user_id","name");
	$author=substr($author,0,MAX_AUTHOR_NAME);
	$text=zstd_compress($text);

	// Publish the post
	redis()->hmset("f:$first_id:f",
		"uid",$user_id,
		"title",$title,
		"text",$text,
		"ut",date(DATE_ATOM),
		"author",$author,
		"name",$name,
	);

	// Register the post in the forum
	redis()->zadd("w:$where",time(),$first_id);

	// Add to the user first post list
	redis()->lpush("u:{$user_id}:f",$first_id);

	// Give points to the user for the first post
	redis()->hmset("r:$first_id:d",
		"pts",POINTS_GIVEN['publish'],
		"vote",0,
		"comm",0,
		"w",$where,
		"ct",date(DATE_ATOM),
	);

	// Update the rank of the post
	updRank($first_id);

	header("Content-Type: application/json",true);
	echo json_encode($first_id);
	exit;
}

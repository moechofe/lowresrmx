<?php // Related to updating the rank of entries in the forum.

function updRank(string $first_id):int
{
	$members=hgetall(redis()->hgetall("r:$first_id:d"));
	$where=$members['w'];
	$ut=date_create($members['ut']);
	// fallback from old version of the rank system
	if(empty($ut)) $ut=date_create($members['ct']);

	$diff=date_diff($ut,date_create());
	$age=(int)$diff->days*24+(int)$diff->h;

	// Compute the points
	$points=0
	+POINTS_GIVEN['publish']
	+POINTS_GIVEN['upvote']*$members['vote']
	+POINTS_GIVEN['comment']*$members['comm']
	;

	// Compute the rank
	$rank=($points-1)/(pow($age+2,1.8));
	if ($rank<0.0001) $rank=0;

	// Update the score
	redis()->hset("r:$first_id:d","pts",$points);

	$name=redis()->hget("f:$first_id:f","name");

	// Update the rank
	if(!empty($name)) redis()->zadd("r:all",$rank,$first_id);
	redis()->zadd("r:$where",$rank,$first_id);

	return $points;
}

// API that return a list of ranked programs as a JSON array.
if(preg_match('/^\/ranked$/',$urlPath)&&$isGet)
{
	$where=@$_GET['w'];
	if(!in_array($where,PROGRAM_VALID_FORUM)) $where='all';

	// TODO: handle more than 99 programs.
	$list=redis()->zrevrange ("r:$where",0,99);

	$published=[];

	// traverse the list of published programs and gather information on each program.
	for($i=0;$i<count($list);++$i)
	{
		$first_id=$list[$i];

		list($program_id,$title,$author,$ut,$name)=$prg=redis()->hmget("f:$first_id:f","pid","title","author","ut","name");
		if(empty($title)||empty($author)||empty($ut)) { cleanInvalidFirst($first_id); continue; }
		$points=redis()->hget("r:$first_id:d","pts");
		$comm=redis()->hget("r:$first_id:d","comm");
		$published[]=[
			'eid'=>$first_id,
			'pid'=>$program_id,
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

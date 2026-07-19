<?php // Related to updating the rank of entries in the forum.

function updRank(string $first_id):int
{
	$members=hgetall(redis()->hgetall("r:$first_id:d"));
	$where=$members['w'];

	// Time term: the FIXED creation time, so the score only changes when the
	// points change (Reddit-style "hot"). Migrate legacy entries that only
	// stored "ut" by pinning their creation time once.
	$ct=$members['ct']??null;
	if(empty($ct))
	{
		$ct=$members['ut']??date(DATE_ATOM);
		redis()->hset("r:$first_id:d","ct",$ct);
	}
	$t=strtotime($ct)-RANK_EPOCH;

	// Compute the points
	$points=0
	+POINTS_GIVEN['publish']
	+POINTS_GIVEN['upvote']*$members['vote']
	+POINTS_GIVEN['comment']*$members['comm']
	;

	// Compute the rank: log-magnitude of points plus a fixed time offset. This
	// value is independent of "now", so it never needs decay recomputation.
	$rank=log10(max($points,1))+$t/RANK_WINDOW;

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

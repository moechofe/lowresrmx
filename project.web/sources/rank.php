<?php // Related to updating the rank of entries in the forum.

function updRank(string $first_id):int
{
	$members=hgetall(redis()->hgetall("r:$first_id:d"));
	$where=$members['w'];
	$ct=date_create($members['ct']);

	$diff=date_diff($ct,date_create());
	$age=(int)$diff->days*24+(int)$diff->h;

	// Compute the points
	$points=0
	+POINTS_GIVEN['publish']
	+POINTS_GIVEN['upvote']*$members['vote']
	+POINTS_GIVEN['comment']*$members['comm']
	;
	// Compute the rank
	$rank=($points-1)/(pow($age+2,1.8));

	// Update the score
	redis()->hset("r:$first_id:d","pts",$points);

	// Update the rank
	redis()->zadd("r:all",$rank,$first_id);
	redis()->zadd("r:$where",$rank,$first_id);

	return $points;
}

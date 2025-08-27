<?php // Related to updating the rank of entries in the forum.

function updRank(string $first_id):void
{
	$members=hgetall(redis()->hgetall("f:$first_id:r"));
	$where=$members['where'];
	$ct=date_create($members['ct']);
	$diff=date_diff($ct,date_create());
	$age=(int)$diff->days*24+(int)$diff->h;
	$points=0
	+POINTS_GIVEN['publish']
	+POINTS_GIVEN['view']*$members['view']
	+POINTS_GIVEN['play']*$members['play']
	+POINTS_GIVEN['comment']*$members['comment']
	+POINTS_GIVEN['upvote']*$members['upvote']
	;
	redis()->hset("f:$first_id:r","point",$points);
	$rank=($points-1)/(pow($age+2,1.8));
	redis()->zadd("rank:all",$rank,$first_id);
	redis()->zadd("rank:$where",$rank,$first_id);
}

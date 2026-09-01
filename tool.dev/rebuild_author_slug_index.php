<?php // Backfill "a:SLUG" -> USER_ID and u:USER_ID["slug"] for every existing user.

require_once __DIR__.'/../project.web/sources/common.php';

$cursor="0";
$total=0;
$skipped=0;
do
{
	list($cursor,$list)=redis()->scan($cursor,"match","u:*","count",1000);
	foreach($list as $key)
	{
		// only the profile hash "u:<uid>", not "u:<uid>:p", "u:<uid>:f", ...
		if(!preg_match('/^u:[^:]+$/',$key)) continue;
		$user_id=substr($key,2);
		$slug=ensureAuthorSlug($user_id);
		if($slug==="")
		{
			$skipped++;
			fwrite(STDOUT,"$user_id\tno usable author name\n");
			continue;
		}
		$total++;
		fwrite(STDOUT,"$user_id\t/~$slug\n");
	}
}
while($cursor!="0");

fwrite(STDERR,"Indexed $total users ($skipped without a usable author name).\n");

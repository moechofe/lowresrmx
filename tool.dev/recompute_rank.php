<?php
$urlPath='';
$isGet=false;
$isPost=false;

require_once __DIR__.'/../project.web/sources/common.php';
require_once __DIR__.'/../project.web/sources/rank.php';

$client=new Client(REDIS_DSN);

$cursor="0";
$total=0;
$skipped=0;
do
{
	list($cursor,$list)=$client->scan($cursor,"match","f:*:f","count",1000);
	foreach($list as $key)
	{
		// keys look like "f:<first_id>:f"
		$first_id=explode(":",$key)[1];

		// Only entries with rank data; malformed/partial posts have no r:…:d
		// and calling updRank on them would create junk "r:" keys.
		if(!$client->exists("r:$first_id:d")) { $skipped++; continue; }

		$points=updRank($first_id);
		$total++;
		fwrite(STDOUT,"$first_id\tpts=$points\n");
	}
}
while($cursor!="0");

fwrite(STDERR,"Recomputed $total entries ($skipped skipped).\n");

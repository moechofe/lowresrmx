<?php

require_once __DIR__.'/redis.php';

defined('WEBSITE_URL') or define('WEBSITE_URL','http://127.0.0.1:8080');
defined('REDIS_DSN') or define('REDIS_DSN','tcp://127.0.0.1:6379');
$client=new Client(REDIS_DSN);

?>
<?xml version="1.0" encoding="UTF-8"?>
<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">
<?php
foreach([
	["community.html","daily","1.0"],
	["show.html","daily","0.8"],
	["chat.html","daily","0.8"],
	["help.html","daily","0.8"],
	["documentation.html","weekly","0.4"],
	["privacy-policy.html","monthly","0.1"],
	["terms-of-service.html","monthly","0.1"],
]as[$path,$freq,$prio]):

$hash=sha1(file_get_contents(WEBSITE_URL."/$path"));
$date=$client->get("m:$hash");
if(!$date)
{
	$date=date('Y-m-d');
	$client->set("m:$hash",$date);
}

?>
	<url>
		<loc>https://ret.ro.it/<?=$path?></loc>
		<changefreq><?=$freq?></changefreq>
		<priority><?=$prio?></priority>
		<lastmod><?=$date?></lastmod>
	</url>
<?php
endforeach;

list($cursor,$list)=$client->scan("0","match","f:*:f");
while($cursor!=="0"):
	foreach($list as $key):

		$first_id=explode(":",$key)[1];
		$ut=$client->hget("$key","ut");
		$time=strtotime($ut);
		$date=date("Y-m-d",$time);
		if(empty($first_id) || empty($ut)) continue;
?>
	<url>
		<loc>https://ret.ro.it/<?=rawurlencode($first_id)?>.html</loc>
		<changefreq><?=(time()-$time<60*60*24*7)?"weekly":"monthly"?></changefreq>
		<priority>0.6</priority>
		<lastmod><?=$date?></lastmod>
	</url>
<?php
	endforeach;
	list($cursor,$list)=$client->scan($cursor,"match","f:*:f");
endwhile;

$cursor="0";
do
{
	list($cursor,$list)=$client->scan($cursor,"match","a:*","count",1000);
	foreach($list as $key):
		$slug=substr($key,2);
		if($slug==="") continue;
		$user_id=$client->get($key);
		if(empty($user_id)) continue;
?>
	<url>
		<loc>https://ret.ro.it/~<?=rawurlencode($slug)?></loc>
		<changefreq>weekly</changefreq>
		<priority>0.5</priority>
		<lastmod><?=date('Y-m-d')?></lastmod>
	</url>
<?php
	endforeach;
}
while($cursor!=="0");
?>
</urlset>

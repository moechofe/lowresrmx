<?php
require_once __DIR__.'/../project.web/sources/common.php';
$date=date('Y-m-d');
?>
<?xml version="1.0" encoding="UTF-8"?>
<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">
<?php
foreach([
	["community.html","weekly","1.0"],
	["show.html","weekly","0.8"],
	["chat.html","weekly","0.8"],
	["help.html","weekly","0.8"],
	["documentation.html","monthly","0.4"],
	["privacy-policy.html","yearly","0.1"],
	["terms-of-service.html","yearly","0.1"],
]as[$path,$freq,$prio]):
?>
	<url>
		<loc>https://ret.ro.it/<?=$path?></loc>
		<changefreq><?=$freq?></changefreq>
		<priority><?=$prio?></priority>
		<lastmod><?=$date?></lastmod>
	</url>
<?php
endforeach;

$client=new Client(REDIS_DSN);
list($cursor,$list)=$client->scan("0","match","f:*:f");
while($cursor!=="0"):
	foreach($list as $key):

		$first_id=explode(":",$key)[1];
		$ut=$client->hget("$key","ut");
		$time=strtotime($ut);
		$date=date("Y-m-d",$time);
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
?>
</urlset>

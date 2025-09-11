<?php

require_once __DIR__.'/common.php';
require_once __DIR__.'/rank.php';

$client=new Client(REDIS_DSN);
$cursor=intval(@file_get_contents(REDIS_SCAN_CURSOR_FILE),10);

list($new_cursor,$list)=$client->scan($cursor,"match","f:*:f");

foreach($list as $key)
{
	$first_id=explode(":",$key)[1];
	updRank($first_id);
}

file_put_contents(REDIS_SCAN_CURSOR_FILE,$new_cursor);

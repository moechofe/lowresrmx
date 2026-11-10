<?php

require_once __DIR__.'/redis.php';

defined('CONTENT_FOLDER') or define('CONTENT_FOLDER',__DIR__.'/../project.web/contents/');
defined('REDIS_DSN') or define('REDIS_DSN','tcp://127.0.0.1:6379');

$redis=new Client(REDIS_DSN);
if(getenv('REDISCLI_AUTH')) $redis->auth(getenv('REDISCLI_AUTH'));

$url=rawurldecode(strval(@$argv[1]));

if(!preg_match('/\/([a-z\']+-[a-z\']+-[0-9a-f]{16})\b/',$url,$matches)) goto usage;
if(empty($matches[1])) goto usage;

$first_id=$matches[1];
$user_id=$redis->hget("f:$first_id:f","uid");

// first entry, comments sequence, comments
$cursor="0";
do
{
	list($cursor,$list)=$redis->scan($cursor,"match","f:$first_id:*");
	foreach($list as $item) echo "redis-cli del \"$item\"\n";
}
while($cursor!="0");

// forum list
foreach(["w:chat","w:help","w:show"] as $where)
	echo "redis-cli zrem \"$where\" \"$first_id\"\n";

// ranked list
foreach(["r:all","r:chat","r:help","r:show"] as $where)
	echo "redis-cli zrem \"$where\" \"$first_id\"\n";

// user's votes
echo "redis-cli del \"r:$first_id:v\"\n";

// score details
echo "redis-cli del \"r:$first_id:d\"\n";

// user's first list
if(!empty($user_id))
	echo "redis-cli lrem \"u:$user_id:f\" 0 \"$first_id\"\n";

// program published by the post: its "first" link and its public files
$pid=$redis->hget("f:$first_id:f","pid");
$first=empty($pid)?null:$redis->hget("p:$pid","first");
if($first===$first_id)
	echo "redis-cli hdel \"p:$pid\" \"first\"\n";

// the files stay when the program was published again in another post
if(empty($first)||$first===$first_id)
{
	$file=empty($pid)?$first_id:$pid;
	$folder=substr($file,0,3);
	echo "rm \"",CONTENT_FOLDER,"$folder/$file.png\"\n";
	echo "rm \"",CONTENT_FOLDER,"$folder/$file.rmx\"\n";
}

exit(0);

usage:
fwrite(STDERR,<<<END
Usage: remove_post URL

END
);

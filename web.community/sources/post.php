<?php // TODO: seems not used

require_once __DIR__.'/common.php';

function nextSyllabe(string $hash,int $i,int &$syllabe):string
{
	$syllabe=($syllabe+ord($hash[$i]))%count(SYLLABLE_LIST);
	return SYLLABLE_LIST[$syllabe];
}

if($url['path']=='/post')
{
	$user_id=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");

	$json=json_decode(file_get_contents('php://input'),true);

	$title=mb_substr(@trim(@$json['title']),0,MAX_POST_TITLE);
	if(empty($title)) badRequest("Fail to read title");

	$text=mb_substr(@trim(@$json['text']),0,MAX_POST_TEXT);
	if(empty($text)) badRequest("Fail to read text");

	$where=@$json['where'];
	if(!in_array($where,FORUM_WHERE)) badRequest("Fail to read where");

	$sequence=redis()->incr("seq:token");
	$uptoken=bin2hex(sodium_crypto_shorthash(strval($sequence),ENTRY_TOKEN_KEY));

	$author=redis()->hget("u:$user_id","name");
	$author=substr($author,0,MAX_AUTHOR_NAME);

	$text=zstd_compress($text);

	redis()->hmset("f:$uptoken",
		"uid",$user_id,
		"title",$title,
		"text",$text,
		"ct",date(DATE_ATOM),
		"author",$author,
	);

	redis()->zadd("w:$where",SCORE_FOR_FIRST_POST,$uptoken);

	redis()->lpush("u:{$user_id}:f",$uptoken);

	header("Content-Type: application/json",true);
	echo json_encode($uptoken);
	exit;
}


<?php // Emoji reactions on posts and comments.

require_once __DIR__.'/common.php';
require_once __DIR__.'/token.php';

function validReaction(string $e):bool
{
	if($e==='' || strlen($e)>64) return false;
	// Exactly one grapheme cluster (only when the intl extension is available).
	if(function_exists('grapheme_strlen'))
	{
		$n=grapheme_strlen($e);
		if($n===false || $n!==1) return false;
	}
	// Only emoji-related codepoints, and at least one pictographic.
	if(!preg_match('/^[\p{Extended_Pictographic}\x{FE0F}\x{200D}\x{1F3FB}-\x{1F3FF}\x{1F1E6}-\x{1F1FF}\x{20E3}#*0-9]+$/u',$e)) return false;
	if(!preg_match('/\p{Extended_Pictographic}/u',$e)) return false;
	return true;
}

function readReactions(string $base,string $user_id):array
{
	$emojis=redis()->smembers("x:$base");
	$out=[];
	foreach($emojis as $e)
	{
		$n=intval(redis()->scard("x:$base:v:$e"));
		if($n<=0) continue;
		$out[]=[
			"e"=>$e,
			"n"=>$n,
			"mine"=>($user_id!=="" && redis()->sismember("x:$base:v:$e",$user_id))?true:false,
		];
	}
	return $out;
}

function toggleReaction(string $base,string $emoji,string $user_id):bool
{
	$key="x:$base:v:$emoji";
	if(redis()->sismember($key,$user_id))
	{
		redis()->srem($key,$user_id);
		// Drop the emoji from the target index once nobody reacts with it anymore.
		if(intval(redis()->scard($key))===0) redis()->srem("x:$base",$emoji);
	}
	else
	{
		$known=redis()->sismember("x:$base",$emoji)?true:false;
		if(!$known && intval(redis()->scard("x:$base"))>=MAX_REACTION_KINDS) return false;
		redis()->sadd($key,$user_id);
		redis()->sadd("x:$base",$emoji);
	}
	return true;
}

function handleReact(string $base):void
{
	list($user_id,$csrf_token)=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");
	if(!validateCSRF($csrf_token)) forbidden("Fail to read token");
	if(!checkRateLimit('react',$user_id)) tooManyRequests("Fail to respect limit");

	$emoji=@trim(@file_get_contents('php://input'));
	if(!validReaction($emoji)) badRequest("Fail to read emoji");

	if(!toggleReaction($base,$emoji,$user_id)) badRequest("Too many reactions");

	header("Content-Type: application/json",true);
	header("X-Robots-Tag: noindex",true);
	echo json_encode(["reactions"=>readReactions($base,$user_id)]);
	exit;
}

// API to toggle an emoji reaction on a post (first entry).
if(preg_match("/^\/($MATCH_ENTRY_TOKEN)\/react$/",$urlPath,$matches)&&$isPost)
{
	$first_id=$matches[1];
	if(!$first_id) badRequest("Fail to read entry");
	if(!redis()->exists("f:$first_id:f")) badRequest("Fail to validate entry");
	handleReact($first_id);
}

// API to toggle an emoji reaction on a comment.
if(preg_match("/^\/($MATCH_ENTRY_TOKEN)\/(\d+)\/react$/",$urlPath,$matches)&&$isPost)
{
	$first_id=$matches[1];
	$cid=intval($matches[2]);
	if(!$first_id) badRequest("Fail to read entry");
	if(!redis()->exists("f:$first_id:$cid")) badRequest("Fail to validate comment");
	handleReact("$first_id:$cid");
}

<?php

require_once __DIR__.'/common.php';

if(preg_match('/\/author$/',$urlPath)&&$isPost)
{
	list($user_id,$csrf_token)=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");
	if(!validateCSRF($csrf_token)) forbidden("Fail to read token");
	if(!checkRateLimit('setting',$user_id)) tooManyRequests("Fail to respect limit");

	$author=file_get_contents('php://input');
	if(empty($author)) badRequest("Fail to read author");
	if(strlen($author)<4||strlen($author)>MAX_AUTHOR_NAME) badRequest("Fail to read author");

	$slug=slugifyAuthor($author);
	if($slug==="") badRequest("Fail to read author");
	if(!claimAuthorSlug($user_id,$slug)) conflict("Fail to claim author name");

	redis()->hset("u:$user_id","author",$author);

	header("Content-Type: application/json",true);
	header("X-Robots-Tag: noindex", true);
	echo json_encode(["slug"=>$slug]);
	exit;
}

if(preg_match('/\/delete_everything$/',$urlPath)&&$isPost)
{
	list($user_id,$csrf_token)=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");
	if(!validateCSRF($csrf_token)) forbidden("Fail to read token");
	if(!checkRateLimit('setting',$user_id)) tooManyRequests("Fail to respect limit");

	redis()->lpush("adm:de",$user_id);
	track('account_delete');

	foreach(redis()->lrange("u:$user_id:s",0,-1) as $session_id)
		redis()->hset("s:$session_id","status","revoked");

	setcookie(
		HEADER_SESSION_COOKIE,"",
		[
			'expires'=>time()-3600,
			'path'=>'/',
			'domain'=>'',
			'secure'=>$isHttps,
			'httponly'=>true,
			'samesite'=>'Lax' // Lax (not Strict) so the session cookie is sent when returning from the OAuth provider
		]);

	header("Location: /community.html",true,302);
	exit;
}

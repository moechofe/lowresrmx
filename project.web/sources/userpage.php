<?php // Echo the HTML for a public user page listing the programs a user published.

require_once __DIR__.'/common.php';

if(preg_match('/^\/~([^\/]{1,'.MAX_AUTHOR_NAME.'})$/u',$urlPath,$matches))
{
	if(!$isGet) { header("HTTP/1.1 404 Not Found",true,404); exit; }

	// Canonicalize: "/~Info Dev" and "/~INFO-DEV" both redirect to "/~info-dev".
	$slug=slugifyAuthor($matches[1]);
	if($slug==="") { header("HTTP/1.1 404 Not Found",true,404); exit; }
	if($slug!==$matches[1])
	{
		header("Status: 301 Moved Permanently",true,301);
		header("Location: /~".rawurlencode($slug));
		exit;
	}

	$user_id=redis()->get("a:$slug");
	if(empty($user_id))
	{
		header("HTTP/1.1 404 Not Found",true,404);
		header("X-Robots-Tag: noindex",true);
		exit;
	}

	$author=redis()->hget("u:$user_id","author");
	if(empty($author)) { header("HTTP/1.1 404 Not Found",true,404); exit; }

	// Newest first: publish/post use lpush on u:USER_ID:f.
	// TODO: handle more than 99 posts.
	$list=redis()->lrange("u:$user_id:f",0,99);

	$published=[];
	foreach($list as $first_id)
	{
		list($program_id,$title,$ut,$name,$status)=redis()->hmget("f:$first_id:f","pid","title","ut","name","status");
		// Program posts only: text-only topics have no "name".
		if(empty($title)||empty($ut)||empty($name)) continue;
		if($status==="banned"||$status==="unlisted") continue;
		$published[]=[
			'eid'=>$first_id,
			'pid'=>$program_id?:$first_id,
			'title'=>$title,
			'ut'=>$ut,
			'points'=>intval(redis()->hget("r:$first_id:d","pts")),
			'comm'=>intval(redis()->hget("r:$first_id:d","comm")),
		];
	}

	// Social-card image: newest published program thumbnail, site logo when the user has none.
	$og_image=count($published)?"/".rawurlencode($published[0]['pid']).".png":"/logo-colored.png";

	track('view:userpage');
	require_once __DIR__.'/userpage.html';
	exit;
}

<?php // Called from javascript to delete a shared program, not published yet.

require_once __DIR__.'/common.php';

if($url['path']==='/delete'&&$_SERVER['REQUEST_METHOD']==='POST')
{
	error_log(__FILE__);

	$user_id=validateSessionAndGetUserId();
	if(!$user_id) forbidden("Fail to read user");

	$json=json_decode(file_get_contents('php://input'),true);

	{
		$program_id=@trim(@$json['p']);
		if(empty($program_id)) badRequest("Fail to read program");
		if(strlen($program_id)<=16||strlen($program_id)>=512) badRequest("Fail to read program");

		redis()->del("p:$program_id");
		redis()->lrem("u:{$user_id}:p",1,$program_id);
	}

	exit;
}

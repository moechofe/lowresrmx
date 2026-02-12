<?php // Used to download a program or a thumbnail.

require_once __DIR__.'/common.php';
require_once __DIR__.'/token.php';

if(preg_match("/^\/($MATCH_ENTRY_TOKEN)\.(".PRG_EXT."|".IMG_EXT.")$/",urldecode($urlPath),$matches)&&$isGet)
{
	$entry_token=$matches[1];
	$extension=$matches[2];

	$folder=substr($entry_token,0,3);
	$path=CONTENT_FOLDER."$folder/$entry_token.$extension";

	$real_path=realpath($path);
	$real_content_folder=realpath(CONTENT_FOLDER);

	// already shared, use the files
	if($real_path && $real_content_folder && strpos($real_path,$real_content_folder)===0)
	{
		header("Content-Type: ".CONTENT_TYPE_MAP[$extension]);
		if ($extension === PRG_EXT) header("X-Robots-Tag: noindex", true);
		readfile($real_path);
		exit;
	}

	// not shared yet
	else
	{
		list($user_id,$csrf_token)=validateSessionAndGetUserId();
		if(!$user_id) forbidden("Fail to read user");
		if(!validateCSRF($csrf_token)) forbidden("Fail to read token");

		// output the file from Redis
		if(redis()->exists("p:{$entry_token}"))
		{
			$file=redis()->hget("p:{$entry_token}",[PRG_EXT=>'prg',IMG_EXT=>'img'][$extension]);
			if($extension===PRG_EXT) $file=zstd_uncompress($file);
			header("Content-Type: ".CONTENT_TYPE_MAP[$extension]);
			if ($extension === PRG_EXT) header("X-Robots-Tag: noindex", true);
			echo $file;
			exit;
		}

		header("Status: 404 Not Found",true,404);
		exit;
	}
}

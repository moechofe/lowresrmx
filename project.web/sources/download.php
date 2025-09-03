<?php // Used to download a program or a thumbnail.

require_once __DIR__.'/common.php';
require_once __DIR__.'/token.php';

if(preg_match("/^\/($MATCH_ENTRY_TOKEN)\.(".PRG_EXT."|".IMG_EXT.")$/",urldecode($urlPath),$matches)&&$isGet)
{
	header("Content-Type: ".CONTENT_TYPE_MAP[$info['extension']]);

	$folder=substr($info['filename'],0,3);
	$path=CONTENT_FOLDER."$folder/{$info['filename']}.{$info['extension']}";
	error_log("Path: $path");

	// already shared, use the files
	if(file_exists($path))
	{
		readfile(CONTENT_FOLDER."$folder/{$info['filename']}.{$info['extension']}");
		exit;
	}

	// not shared yet
	else
	{
		$user_id=validateSessionAndGetUserId();
		if(!$user_id) forbidden("Fail to read user");

		// output the file from Redis
		if(redis()->exists("p:{$info['filename']}"))
		{
			$file=redis()->hget("p:{$info['filename']}",[PRG_EXT=>'prg',IMG_EXT=>'img'][$info['extension']]);
			if($info['extension']===PRG_EXT) $file=zstd_uncompress($file);
			echo $file;
			exit;
		}

		header("Status: 404 Not Found",true,404);
		exit;
	}
}

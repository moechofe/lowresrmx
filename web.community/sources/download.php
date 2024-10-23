<?php

require_once __DIR__.'/common.php';

if(strlen($info['filename'])>16&&strlen($info['filename'])<512&&in_array($info['extension'],['rmx','png']))
{
	header("Content-Type: ".[
		'rmx'=>'text/plain',
		'png'=>'image/png',
	][$info['extension']]);

	$folder=substr($info['filename'],0,3);
	$path=CONTENT_FOLDER."$folder/{$info['filename']}.{$info['extension']}";
	error_log("Path: $path");

	if(file_exists($path))
	{
		readfile(CONTENT_FOLDER."$folder/{$info['filename']}.{$info['extension']}");
		exit;
	}
	else
	{
		header("Status: 404 Not Found",true,404);
		exit;
	}
}


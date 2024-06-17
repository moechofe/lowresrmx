<?php

require_once __DIR__.'/common.php';

if(strlen($info['basename'])>16&&strlen($info['basename'])<512&&preg_match('/\b([0-9a-fA-F]{16})\.html$/',$info['basename'],$matches))
{
	$token=$matches[1];

	list($title,$text)=redis()->hmget("f:$token","title","text");
	$text=zstd_uncompress($text);

	require_once __DIR__.'/entry.html';
	exit;
}

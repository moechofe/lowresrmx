<?php // Main entry point for the community web app. All requests go through here.

// order is important
// TODO: should be required in every file that needs it
require_once __DIR__.'/common.php';
require_once __DIR__.'/token.php';

require_once __DIR__.'/sign.php';

require_once __DIR__.'/player.php';

require_once __DIR__.'/upload.php';
require_once __DIR__.'/download.php';

require_once __DIR__.'/share.php';
require_once __DIR__.'/list.php';
require_once __DIR__.'/delete.php';

require_once __DIR__.'/publish.php';
require_once __DIR__.'/post.php';
require_once __DIR__.'/entry.php';
require_once __DIR__.'/comment.php';
require_once __DIR__.'/score.php';

// server static files
if(in_array($urlPath,[
	'/community.html',
	'/community.js',
	'/community.css',
	'/show.html',
	'/show.js',
	'/show.css',
	'/chat.html',
	'/chat.js',
	'/chat.css',
	'/help.html',
	'/help.js',
	'/help.css',
	'/player.js',
	'/entry.css',
	'/entry.js',
	'/share.html',
	'/share.js',
	'/share.css',
]))
{
	header("Content-Type: ".[
		'html'=>'text/html',
		'css'=>'text/css',
		'js'=>'text/javascript',
	][$info['extension']]);
	require_once __DIR__.$urlPath;
	exit;
}
if(in_array($urlPath,[
	'/pico.min.css',
	'/player.wasm',
	'/favicon.ico',
	'/logo.png',
]))
{
	header("Content-Type: ".[
		'wasm'=>'application/wasm',
		'ico'=>'image/x-icon',
		'png'=>'image/png',
		'css'=>'text/css'
	][$info['extension']]);
	readfile(__DIR__.$urlPath);
	exit;
}

if($urlPath==='/')
{
	header("Status: 301 Moved Permanently",true,301);
	header("Location: community.html");
	exit();
}

header("Status: 404 Not Found",true,404);
exit();

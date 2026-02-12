<?php // Main entry point for the community web app. All requests go through here.

// order is important
// TODO: should be required in every file that needs it
require_once __DIR__.'/common.php';
require_once __DIR__.'/token.php';

require_once __DIR__.'/sign.php';
require_once __DIR__.'/setting.php';

require_once __DIR__.'/player.php';

require_once __DIR__.'/upload.php';
require_once __DIR__.'/download.php';

require_once __DIR__.'/share.php';
require_once __DIR__.'/list.php';

require_once __DIR__.'/entry.php';
require_once __DIR__.'/comment.php';
require_once __DIR__.'/score.php';

// server static files maybe
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
	'/entry.css',
	'/entry.js',
	'/share.html',
	'/share.js',
	'/share.css',
	'/setting.html',
	'/setting.js',
	'/setting.css',
	'/about.html',
	'/about.js',
	'/about.css',
	// '/sign-in.html', // in DEV only
]))
{
	header("Content-Type: ".[
		'html'=>'text/html',
		'css'=>'text/css',
		'js'=>'application/javascript',
	][$info['extension']]);
	if($isProd) readfile(__DIR__.$urlPath);
	else require_once __DIR__.$urlPath;
	exit;
}

// server static files
if(in_array($urlPath,[
	'/pico.min.css',
	'/sign-in.css',
	'/sign-in.js',
	'/player.wasm',
	'/player.css',
	'/player.js',
	'/favicon.ico',
	'/logo-white.png',
	'/logo-colored.png',
	'/privacy-policy.html',
	'/terms-of-service.html',
	'/documentation.html',
	'/documentation.css',
	'/robots.txt',
	'/sitemap.xml',
]))
{
	header("Content-Type: ".[
		'html'=>'text/html',
		'wasm'=>'application/wasm',
		'ico'=>'image/x-icon',
		'png'=>'image/png',
		'css'=>'text/css',
		'js'=>'application/javascript',
		'txt'=>'text/plain',
		'xml'=>'application/xml',
	][$info['extension']]);
	readfile(__DIR__.$urlPath);
	exit;
}

if($urlPath==='/info' && !$isProd)
{
	phpinfo();
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

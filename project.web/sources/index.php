<?php // Main entry point for the community web app. All requests go through here.

// order is important
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
require_once __DIR__.'/reaction.php';
require_once __DIR__.'/notification.php';

require_once __DIR__.'/image.php';

// server PHP generated files
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
	'/privacy-policy.html',
	'/terms-of-service.html',
	'/documentation.html',
	'/documentation.js',
]))
{
	header("Content-Type: ".[
		'html'=>'text/html',
		'css'=>'text/css',
		'js'=>'application/javascript',
	][$info['extension']]);
	if($info['extension']==='html') track('view:'.$info['filename']);
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
	'/emoji-picker-element.js',
	'/emoji-data.json',
	'/favicon.ico',
	'/logo-white.png',
	'/logo-colored.png',
	'/documentation.css',
	'/documentation-www.css',
	'/robots.txt',
	'/sitemap.xml',
	'/04.webp',
	'/05.webp',
	'/06.webp',
	'/medium-zoom.min.js',
]))
{
	header("Content-Type: ".[
		'html'=>'text/html',
		'wasm'=>'application/wasm',
		'ico'=>'image/x-icon',
		'png'=>'image/png',
		'webp'=>'image/webp',
		'css'=>'text/css',
		'js'=>'application/javascript',
		'json'=>'application/json',
		'txt'=>'text/plain',
		'xml'=>'application/xml',
	][$info['extension']]);
	header("Content-Length: ".filesize(__DIR__.$urlPath));
  header("X-Content-Type-Options: nosniff");
	$etag='"'.filesize(__DIR__.$urlPath).'-'.filemtime(__DIR__.$urlPath).'"';
	header("ETag: ".$etag);
	header("X-Content-Type-Options: nosniff");
	if(isset($_SERVER['HTTP_IF_NONE_MATCH']) && $_SERVER['HTTP_IF_NONE_MATCH']===$etag)
	{
		header("Status: 304 Not Modified",true,304);
		exit;
	}
	header("Content-Length: ".filesize(__DIR__.$urlPath));
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

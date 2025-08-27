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
require_once __DIR__.'/last_shared.php';
require_once __DIR__.'/publish.php';
require_once __DIR__.'/delete.php';

require_once __DIR__.'/list_first.php';
require_once __DIR__.'/entry.php';
require_once __DIR__.'/comment.php';
require_once __DIR__.'/score.php';






// require_once __DIR__.'/published.php';

// require_once __DIR__.'/player.php';

// require_once __DIR__.'/trends.php';
// require_once __DIR__.'/my_programs.php';
// require_once __DIR__.'/program.php';

// require_once __DIR__.'/post.php';
// require_once __DIR__.'/entry.php';







// server static files
if(in_array($url['path'],[
	'/community.html',
	'/community.js',
	'/community.css',
	'/post.css',
	'/post.js',
	'/pico.min.css',
	'/player.js',
	'/player.wasm',
	'/entry.css',
	'/entry.js',
	'/share.html',
	'/share.js',
	'/share.css',
	'/last_shared.html',
	'/last_shared.js',
]))
{
	header("Content-Type: ".[
		'html'=>'text/html',
		'css'=>'text/css',
		'js'=>'text/javascript',
		'wasm'=>'application/wasm',
	][$info['extension']]);
	require_once __DIR__.$url['path'];
	exit;
}

if($url['path']==='/')
{
	header("Status: 301 Moved Permanently",true,301);
	header("Location: community.html");
	exit();
}

header("Status: 404 Not Found",true,404);
exit();

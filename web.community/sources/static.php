<?php

require_once __DIR__.'/common.php';

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

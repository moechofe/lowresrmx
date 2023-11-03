<?php

require_once __DIR__.'/common.php';

if(strlen($query)>16&&strlen($query)<512&&$info['basename']=='program.html')
{
	require_once __DIR__.'/program.html';
	exit;
}

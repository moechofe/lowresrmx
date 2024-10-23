<?php

require_once __DIR__.'/common.php';

if(preg_match('/^\/([\w-]*)\/program.html$/',$url['path'],$matches))
{
	require_once __DIR__.'/program.html';
	exit;
}

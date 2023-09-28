<?php

error_reporting(E_ALL);

function badRequest(int $line)
{
	header("HTTP/1.1 400 Bad Request");
	trigger_error("Fail to upload at line $line");
	exit;
}

$program=file_get_contents('php://input');
if(empty($program)) badRequest(__LINE__);

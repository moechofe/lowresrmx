<?php

/*

IDEA:

	publishing increase score
	updating increase score according to follow
	upvote increase score

	score decrease every ...
*/

require_once __DIR__.'/common.php';

if($url['path']==='/trends')
{
	$client=new Client('tcp://127.0.0.1:6379');
	if(!$client) internalServerError("Fail to reach db",__FILE__,__LINE__);

	exit;
}

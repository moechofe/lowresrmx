<?php

require_once __DIR__.'/common.php';

if($url['path']==='/is_signed')
{
	$user_id=validateSessionAndGetUserId();
	echo $user_id?"true":"false";
	exit;
}

<?php

function cleanInvalidFirst($first_id)
{
	$user_id=redis()->hget("f:$first_id:f","uid");
	if($user_id) cleanInvalidUserFirst($user_id, $first_id);

	// TODO: remove from r:WHERE_ID
	// TODO: remove from w:WHERE_ID
	// TODO: remove r:FIRST_ID:v
	// TODO: remove r:ENTRY_TOKEN_ID:d

	// TODO: clean f:ENTRY_TOKEN_ID:s
	// TODO: clean f:ENTRY_TOKEN_ID:CID
	// TODO: clean f:ENTRY_TOKEN_ID:c

}

function cleanInvalidUserFirst($user_id,$first_id)
{
	// TODO: remove from list u:USER_ID:l


}

function cleanInvalidUserProgram($user_id,$program_id)
{
	// TODO: clean p:PROGRAM_ID
	// TODO: clean from u:USER_ID:p
}

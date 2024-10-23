<?php // Reached when user share a program from the iOS app.

require_once __DIR__.'/common.php';

// TODO: check limit

if($url['path']==='/upload'&&$_SERVER['REQUEST_METHOD']==='GET')
{
	error_log(__FILE__);

	// Check for the field from the iOS app
	$program=base64_decode(trim(@$_GET['p']));
	if(empty($program)||strlen($program)>MAX_UPLOAD_PROGRAM) badRequest("Fail to read program");
	$thumbnail=base64_decode(trim(@$_GET['t']));
	if(empty($thumbnail)||strlen($thumbnail)>MAX_UPLOAD_THUMBNAIL) badRequest("Fail to read thumbnail");
	$name=trim(@$_GET['n']);
	// TODO: check size of the name
	if(empty($name)) badRequest("Fail to read name");

	$uptoken=generateUploadToken();
	error_log("Upload token: $uptoken");
	if(empty($uptoken)) badRequest("Fail to generate token header");

	// store the uploaded content
	redis()->hsetnx("t:$uptoken","prg",zstd_compress($program));
	redis()->hsetnx("t:$uptoken","img",zstd_compress($thumbnail));

	// store the name
	redis()->hsetnx("t:$uptoken","name",$name);

	// will expire in 1 hour
	redis()->expire("t:$uptoken",UPLOAD_TOKEN_TTL);

	header("Location: /share?uptoken=$uptoken");
	exit;
}

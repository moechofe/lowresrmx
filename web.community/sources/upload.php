<?php // Reached when user share a program from the iOS app.
// It will store the program and thumbnail in Redis and redirect to /share with the upload token.

require_once __DIR__.'/common.php';

// TODO: check limit

if($url['path']==='/upload'&&$_SERVER['REQUEST_METHOD']==='GET')
{
	// Check for the field from the iOS app
	$program=base64_decode(str_replace('_','/',str_replace('-','+',@trim(@$_GET['p']))));
	if(empty($program)||strlen($program)>MAX_UPLOAD_PROGRAM) badRequest("Fail to read program");
	$thumbnail=base64_decode(str_replace('_','/',str_replace('-','+',@trim(@$_GET['t']))));
	if(empty($thumbnail)||strlen($thumbnail)>MAX_UPLOAD_THUMBNAIL) badRequest("Fail to read thumbnail");
	$name=@trim(@$_GET['n']);
	// TODO: check size of the name
	if(empty($name)) badRequest("Fail to read name");

	$uptoken=generateUploadToken();
	error_log("Upload token: $uptoken");
	if(empty($uptoken)) badRequest("Fail to generate token header");

	// store the uploaded content
	redis()->hsetnx("t:$uptoken","prg",$program); // program is sent already zstd compressed
	redis()->hsetnx("t:$uptoken","img",$thumbnail);

	// store the name
	redis()->hsetnx("t:$uptoken","name",$name);

	// will expire in 1 hour
	redis()->expire("t:$uptoken",UPLOAD_TOKEN_TTL);

	header("Location: /share?uptoken=$uptoken");
	exit;
}

else if($url['path']==='/upload'&&$_SERVER['REQUEST_METHOD']==='POST')
{
	header("Content-Type: application/json",true);
	error_log(__FILE__);

	$file=file_get_contents('php://input');
	if(empty($file)) badRequest("Fail to read file");

	$uptoken=generateUploadToken();
	error_log("Upload token: $uptoken");
	if(empty($uptoken)) badRequest("Fail to generate token header");

	if(substr($file,0,8)==="\x89\x50\x4E\x47\x0D\x0A\x1A\x0A")
	{
		if(strlen($file)>MAX_UPLOAD_THUMBNAIL) badRequest("Fail to read thumbnail");
		redis()->hsetnx("t:$uptoken","img",$file);
	}
	else
	{
		if(strlen($file)>MAX_UPLOAD_PROGRAM) badRequest("Fail to read program");
		$file=zstd_compress($file);
		if(empty($file)) badRequest("Fail to compress file");
		redis()->hsetnx("t:$uptoken","prg",$file);
	}

	// will expire in 1 hour
	redis()->expire("t:$uptoken",UPLOAD_TOKEN_TTL);

	echo json_encode($uptoken);
	exit;
}

<?php // Reached when user share a program from the iOS app.
// It will store the program and thumbnail in Redis and redirect to /share with the upload token.

require_once __DIR__.'/common.php';

if(preg_match('/^\/upload$/',$urlPath)&&$isPost)
{
	if(!checkRateLimit('upload',getClientIP())) tooManyRequests("Fail to respect limit");

	$json=json_decode(file_get_contents('php://input'),true);

	$prg=base64_decode($json['p']);
	if(empty($prg)||strlen($prg)>MAX_UPLOAD_PROGRAM) badRequest("Fail to read program");
	if(substr($prg,0,4)!=="\x28\xB5\x2F\xFD") badRequest("Fail to read program");

	$img=base64_decode($json['i']);
	if(empty($img)||strlen($img)>MAX_UPLOAD_THUMBNAIL) badRequest("Fail to read image");
	if(substr($img,0,8)!=="\x89PNG\r\n\x1a\n") badRequest("Fail to read thumbnail");

	$name=mb_substr(@trim(@$json['n']),0,MAX_POST_NAME);

	$uptoken=generateUploadToken();
	if(empty($uptoken)) badRequest("Fail to generate token header");

	// store the uploaded content
	redis()->hsetnx("t:$uptoken","prg",$prg); // program is sent already zstd compressed
	redis()->hsetnx("t:$uptoken","img",$img);

	// store the name
	redis()->hsetnx("t:$uptoken","name",$name);

	// will expire in 1 hour
	redis()->expire("t:$uptoken",UPLOAD_TOKEN_TTL);

	header("Content-Type: application/json",true);
	echo json_encode($uptoken);
	exit;
}

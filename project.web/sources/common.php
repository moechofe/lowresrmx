<?php // Common functions and constants for the LowResRMX community website.

header_remove('X-Powered-By');

error_reporting(E_ALL);

require_once __DIR__.'/private.php';

defined('WEBSITE_URL') or define('WEBSITE_URL','http://lowresrmx.top:8080');
defined('REDIS_DSN') or define('REDIS_DSN','tcp://127.0.0.1:6379');
defined('APP_SCHEME') or define('APP_SCHEME','lowresrmx:');

const HEADER_TOKEN='X-Application-Token';
const HEADER_FILE_TYPE='X-Application-Type';
const HEADER_SESSION='X-Application-Session'; // TODO: should be removed, I'm using cookies now
const HEADER_SESSION_COOKIE='sid'; // Sauce: https://developer.mozilla.org/en-US/docs/Web/HTTP/Cookies
const HEADER_ADMIN_ACCESS='X-Application-Request';
const HEADER_SCAN_CURSOR='X-Application-Cursor';

const UPLOAD_TOKEN_TTL=60*60; // 1 hour
const LOGIN_GOOGLE_TTL=60*5; // 5 minutes
const LOGIN_DISCORD_TTL=60*5; // 5 minutes
const SESSION_TTL=60*60*24*30; // 30 days

defined('CONTENT_FOLDER') or define('CONTENT_FOLDER',__DIR__.'/../contents/');
const REDIS_SCAN_CURSOR_FILE="./.updrank_cursor";

const SYLLABLE_LIST=['ing','er','a','ly','ed','i','es','re','tion','in','e','con','y','ter','ex','al','de','com','o','di','en','an','ty','ry','u','ti','ri','be','per','to','pro','ac','ad','ar','ers','ment','or','tions','ble','der','ma','na','si','un','at','dis','ca','cal','man','ap','po','sion','vi','el','est','la','lar','pa','ture','for','is','mer','pe','ra','so','ta','as','col','fi','ful','ger','low','ni','par','son','tle','day','ny','pen','pre','tive','car','ci','mo','on','ous','pi','se','ten','tor','ver','ber','can','dy','et','it','mu','no','ple','cu','fac','fer','gen','ic','land','light','ob','of','pos','tain','den','ings','mag','ments','set','some','sub','sur','ters','tu','af','au','cy','fa','im','li','lo','men','min','mon','op','out','rec','ro','sen','side','tal','tic','ties','ward','age','ba','but','cit','cle','co','cov','da','dif','ence','ern','eve','hap','ies','ket','lec','main','mar','mis','my','nal','ness','ning','n\'t','nu','oc','pres','sup','te','ted','tem','tin','tri','tro','up','va','ven','vis','am','bor','by','cat','cent','ev','gan','gle','head','high','il','lu','me','nore','part','por','read','rep','su','tend','ther','ton','try','um','uer','way','ate','bet','bles','bod','cap','cial','cir','cor','coun','cus','dan','dle','ef','end','ent','ered','fin','form','go','har','ish','lands','let','long','mat','meas','mem','mul','ner','play','ples','ply','port','press','sat','sec','ser','south','sun','the','ting','tra','tures','val','var','vid','wil','win','won','work','act','ag','air','als','bat','bi','cate','cen','char','come','cul','ders','east','fect','fish','fix','gi','grand','great','heav','ho','hunt','ion','its','jo','lat','lead','lect','lent','less','lin','mal','mi','mil','moth','near','nel','net','new','one','point','prac','ral','rect','ried','round','row','sa','sand','self','sent','ship','sim','sions','sis','sons','stand','sug','tel','tom','tors','tract','tray','us','vel','west','where','write'];

const MAX_AUTHOR_NAME=100;
const MAX_POST_TITLE=100;
const MAX_POST_NAME=100;
const MAX_POST_TEXT=15000;
const MAX_UPLOAD_PROGRAM=0x10000;
const MAX_UPLOAD_THUMBNAIL=132710;

const PROGRAM_VALID_FORUM=['show','chat','help'];
const TOPIC_VALID_FORUM=['chat','help'];

const PRG_EXT='rmx';
const PRG_CONTENT_TYPE='text/plain';
const IMG_EXT='png';
const IMG_CONTENT_TYPE='image/png';
const CONTENT_TYPE_MAP=[
	PRG_EXT=>PRG_CONTENT_TYPE,
	IMG_EXT=>IMG_CONTENT_TYPE,
];

const POINTS_GIVEN=[
	'comment'=>5,
	'upvote'=>50,
	'publish'=>100,
	'update_after_week'=>30, // maxed out at 90 points
];

const APPLE_APP_STORE="#";

require_once __DIR__.'/redis.php';

function backtrace()
{
	file_put_contents('php://stderr',var_export(debug_backtrace(),true));
}

function badRequest(string $reason):void
{
	header("HTTP/1.1 400 Bad Request",true,400);
	trigger_error($reason);
	backtrace();
	exit;
}

function forbidden(string $reason):void
{
	header("HTTP/1.1 403 Forbidden",true,403);
	trigger_error($reason);
	backtrace();
	exit;
}

function internalServerError(string $reason):void
{
	header("HTTP/1.1 500 Internal Server Error",true,500);
	trigger_error($reason);
	backtrace();
	exit;
}

function redis():Client
{
	static $client=null;
	if($client===null) $client=new Client(REDIS_DSN);
	if(!$client) internalServerError("Fail to reach db");
	return $client;
}

function hgetall(array $data):array
{
	$result=[];
	while($data)
	{
		$k=array_shift($data);
		$result[$k]=array_shift($data);
	}
	return $result;
}

function revokeSession(string $session_id):void
{
	$user_id=redis()->hget("s:$session_id","uid");
	redis()->del("s:$session_id");
	if($user_id) redis()->lrem("u:$user_id:s",0,$session_id);
}

function validateSessionAndGetUserId():string
{
	$session_id=@hex2bin(@$_COOKIE[HEADER_SESSION_COOKIE]);
	if(!$session_id) return "";

	list($status,$user_id)=redis()->hmget("s:$session_id","status","uid");
	error_log("User: ".json_encode($user_id));
	switch($status)
	{
	case "revoked":
		revokeSession($session_id);
		return "";

	case "banned":
		revokeSession($session_id);
		forbidden("User is banned");

	case "allowed": default:
		if(!$user_id) $user_id="";
		return $user_id;
	}
}

function getServerQueryString():string
{
	if(isset($_SERVER['QUERY_STRING'])) return $_SERVER['QUERY_STRING'];
	elseif(isset($_SERVER['REQUEST_URI']))
	{
		$xpl=explode('/',$_SERVER['REQUEST_URI']);
		$base=$xpl[array_key_last($xpl)];
		if(strpos($base,'?')!==false) return substr($base,strpos($base,'?')+1);
	}
	return "";
}

function getQueryParams(string $query):array
{
	$params=[];
	parse_str($query,$params);
	return $params;
}

if(!defined("STDERR") or !posix_isatty(STDERR))
{
	$request=$_SERVER['REQUEST_URI'];
	$url=parse_url($request);
	$urlPath=rawurldecode($url['path']);
	$info=pathinfo($urlPath);
	$query=getServerQueryString();
	$domain=$_SERVER['SERVER_NAME'];
	$params=getQueryParams($query);
	$isGet=$_SERVER['REQUEST_METHOD']==='GET';
	$isPost=$_SERVER['REQUEST_METHOD']==='POST';

	header("Server: lowresrmx",true);

	if($_SERVER['HTTP_HOST']=='localhost' || $_SERVER['HTTP_HOST']=='localhost')
	{
		error_log("");
		error_log("");
		error_log("===============================");
		error_log("Request: ".substr($request,0,40)."... (".strlen($request)." bytes)");
		error_log("Path: ".$url["path"]);
		error_log("Info: ".json_encode($info));
		error_log("Query: ".substr($query,0,40)."... (".strlen($query)." bytes)");
		error_log("Headers: ".json_encode(getallheaders()));
		error_log("Cookie: ".json_encode($_COOKIE));
		error_log("Body: ".file_get_contents('php://input'));
		error_log("Params: ".json_encode(array_keys($params)));
		error_log("Outders: ".json_encode(headers_list()));
		error_log("Server: ".json_encode($_SERVER));
	}
}

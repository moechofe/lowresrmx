<?php

/**
 * Summary of markdown2html
 * @param string $in
 * @param array $options
 * @return bool|string
 *
 * Supported markdown features:
 *
 * \n => new line
 * \n\n => new paragraph
 * * or _ => emphasis
 * ** or __ => strong
 * *** or ___ => strong + emphasis
 * [text](url) => link
 * ![alt](url) => image with figure caption
 * ` => monospace
 * TODO: ``` code block
 * TODO: parse https link
 */

// Only http(s), mailto and scheme-less URLs survive; any other scheme becomes "".
function markdownUrl(string $url):string
{
	$url=trim($url,"\x00..\x20");
	if(preg_match('/^([a-z][a-z0-9+.\-]*):/i',$url,$m) && !in_array(strtolower($m[1]),['http','https','mailto'],true)) return "";
	$url=preg_replace_callback('/[\x00-\x20\x7F]/',fn($m)=>rawurlencode($m[0]),$url);
	return htmlspecialchars($url,ENT_QUOTES,'UTF-8');
}

function markdownLink(bool $img,string $url,string $captured):string
{
	$href=markdownUrl($url);
	return $img?"<figure><img src=\"$href\"><figcaption>$captured</figcaption></figure>":"<a href=\"$href\">$captured</a>";
}

function markdown2html(string $in):String
{
	$in=preg_replace('/\r\n?/',"\n",strip_tags(trim($in)));
	$chars=mb_str_split($in);

	// current parsing state: p, ul, figure, code
	$curr="";
	// current parsing flags:
	$em=false;
	$strong=false;
	$link=false;
	$inUrl=false;
	$img=false;
	$code=false;
	$newline=false;
	$noparse=false;
	// captured content for links and images
	$captured="";

	$len=count($chars);

	ob_start();

	$i=-1;
next:
	if(++$i>=$len)goto close;
	$c=$chars[$i];
retry:
	if($i>=$len)goto end;

	if($c=="\n" && !$newline)
	{
		$newline=true;
		goto next;
	}
	else if($c=="\n" && $newline && $curr!="")
	{
too_newline:
		if(++$i>=$len)goto close;$c=$chars[$i];if($i>=$len)goto end;
		if($c=="\n")goto too_newline;
		goto close;
	}
	else if(($c=="*"||$c=="_") && !$inUrl && !$noparse)
	{
		$count=1;
count_stars:
		if(++$i>=$len)goto close;$c=$chars[$i];if($i>=$len)goto end;
		if($c=="*" || $c=="_"){$count++;goto count_stars;}
		if($curr==""){$curr="p";echo"<p>";}
		if($count==1 && !$em){$em=true;echo"<em>";}
		else if($count==1 && $em){$em=false;echo"</em>";}
		else if($count==2 && !$strong){$strong=true;echo"<strong>";}
		else if($count==2 && $strong){$strong=false;echo"</strong>";}
		else if($count>=3)
		{
			if($em)echo"</em>";
			if(!$strong){$strong=true;echo"<strong>";}
			else{$strong=false;echo"</strong>";}
			if(!$em){$em=true;echo"<em>";}
			else{$em=false;}
		}
		goto retry;
	}
	else if($c=="!" && !$noparse && !$inUrl)
	{
		if(++$i>=$len)goto close;$c=$chars[$i];if($i>=$len)goto end;
		if($c=="[")
		{
			$img=true;
			goto close;
		};
		echo "!";
		goto retry;
	}
	else if($c=="[" && !$link && !$noparse)
	{
		if($newline && $curr!="")echo"<br>";$newline=false;
		if($curr=="" && !$img){$curr="p";echo"<p>";}
		if($img)$curr="figure";
		ob_start();
		$link=true;
		goto next;
	}
	else if($c=="]" && $link && !$inUrl && !$noparse)
	{
		$captured=ob_get_clean();
		if(++$i>=$len)
		{
			echo markdownLink($img,"",$captured);
			if($img){$img=false;$curr="";}
			$link=false;
			goto close;
		}
		$c=$chars[$i];
		if($c=="(")
		{
			ob_start();
			$inUrl=true;
			goto next;
		}
		echo markdownLink($img,"",$captured);
		if($img){$img=false;$curr="";}
		$link=false;
		goto retry;
	}
	else if($c==")" && $inUrl && !$noparse)
	{
		$url=ob_get_clean();
		$inUrl=false;
		echo markdownLink($img,$url,$captured);
		if($img){$img=false;$curr="";}
		$link=false;
		goto next;
	}
	else if($c=='`' && !$code && !$inUrl)
	{
		$count=1;
	count_back_quote:
		if(++$i>=$len)goto close;$c=$chars[$i];if($i>=$len)goto end;
		if($c=="`"){$count++;goto count_back_quote;}
		if($count>=3)
		{
			if($curr=="pre"){echo"</$curr>";$curr="";$noparse=false;goto retry;}
			if($curr!=""){echo"</$curr>";}
			echo"<pre>";
			$curr="pre";
			$noparse=true;
			if($c=="\n") goto next;
			else goto retry;
		}
		if($curr==""){$curr="p";echo"<p>";}
		if(!$noparse){echo "<code>";$code=true;$noparse=true;goto retry;}
		else{echo htmlspecialchars("`");goto retry;}
	}
	else if($c=='`' && $code)
	{
		echo "</code>";
		$code=false;
		$noparse=false;
		goto next;
	}
	else
	{
		if($newline && $curr!="")
		{
			echo $inUrl?"\n":"<br/>";
			$newline=false;
		}
		if($curr==""){$curr="p";echo"<p>";}
		echo $inUrl?$c:htmlspecialchars($c);
		goto next;
	}
	exit;

close:
	if($link){if($inUrl){$url=ob_get_clean();$inUrl=false;}else{$url="";$captured=ob_get_clean();}echo markdownLink($img,$url,$captured);if($img){$img=false;$curr="";}$link=false;}
	if($code){echo"</code>";$code=false;$noparse=false;}
	if($em){echo"</em>";$em=false;}
	if($strong){echo"</strong>";$strong=false;}
	if($curr){echo"</{$curr}>";$curr="";}
	$newline=false;
	goto retry;

end:
	if($link){if($inUrl){$url=ob_get_clean();$inUrl=false;}else{$url="";$captured=ob_get_clean();}echo markdownLink($img,$url,$captured);if($img){$img=false;$curr="";}$link=false;}
	if($em){echo"</em>";$em=false;}
	if($strong){echo"</strong>";$strong=false;}
	if($curr){echo"</{$curr}>";$curr="";}
	$dom=@Dom\HTMLDocument::createFromString(ob_get_clean());
	return substr($dom->saveHtml($dom->body),strlen("<body>"),-strlen("</body>"));
}

function test($tested,$expected)
{
	$candidate=markdown2html($tested);
	if($candidate===$expected && ob_get_level()==0)echo".";
	else
	{
		$level=ob_get_level();
		while(ob_get_level()>0)ob_end_clean();
		echo"\nTest failed:\n";
		echo"Tested:    ",json_encode($tested),"\n";
		echo"Expected:  $expected\n";
		echo"Candidate: $candidate\n";
		echo"Ob level:  $level\n";
	}
}

// test("\n","");
// test("\n\n","");
// test("\nga\n","<p>ga</p>");
// test("🎅","<p>🎅</p>");
// test("ga\nbu","<p>ga<br>bu</p>");
// test("ga\n\nbu","<p>ga</p><p>bu</p>");
// test("ga\n\n\nbu","<p>ga</p><p>bu</p>");
// test("ga *bu* zo","<p>ga <em>bu</em> zo</p>");
// test("ga **bu** zo","<p>ga <strong>bu</strong> zo</p>");
// test("ga ***bu*** zo","<p>ga <strong><em>bu</em></strong> zo</p>");
// test("ga ****bu**** zo","<p>ga <strong><em>bu</em></strong> zo</p>");
// test("ga *bu\nu* zo","<p>ga <em>bu<br>u</em> zo</p>");
// test("ga **bu\n\nzo** meu","<p>ga <strong>bu</strong></p><p>zo<strong> meu</strong></p>"); // ???? is it possible?
// test("*ga ***bu**** zo*","<p><em>ga </em><strong>bu</strong><em> zo</em></p>");
// test("[ga](bu)","<p><a href=\"bu\">ga</a></p>");
// test("zo\n[ga](bu)","<p>zo<br><a href=\"bu\">ga</a></p>");
// test("zo[ga]","<p>zo<a href=\"\">ga</a></p>");
// test("zo(ga)","<p>zo(ga)</p>");
// test("[[ga](bu)","<p><a href=\"bu\">[ga</a></p>");
// test("[ga]](bu)","<p><a href=\"\">ga</a>](bu)</p>");
// test("[ga]((bu)","<p><a href=\"(bu\">ga</a></p>");
// test("[ga](bu))","<p><a href=\"bu\">ga</a>)</p>");
// test("[*ga*](bu)","<p><a href=\"bu\"><em>ga</em></a></p>");
// test("[ga](*bu*)","<p><a href=\"*bu*\">ga</a></p>");
// test("*ga[bu](zo)meu*","<p><em>ga<a href=\"zo\">bu</a>meu</em></p>");
// test("ga!bu","<p>ga!bu</p>");
// test("![ga](bu)","<figure><img src=\"bu\"><figcaption>ga</figcaption></figure>");
// test("zo![ga](bu)meu","<p>zo</p><figure><img src=\"bu\"><figcaption>ga</figcaption></figure><p>meu</p>");
// test("![g\na](bu)","<figure><img src=\"bu\"><figcaption>g<br>a</figcaption></figure>");
// test("![ga](b\nu)","<figure><img src=\"b%0Au\"><figcaption>ga</figcaption></figure>");
// test("![ga](b\tu)","<figure><img src=\"b%09u\"><figcaption>ga</figcaption></figure>");
// test("[ga](##)","<p><a href=\"##\">ga</a></p>");
// test("[ga](??)","<p><a href=\"??\">ga</a></p>");
// test("[ga](&&)","<p><a href=\"&amp;&amp;\">ga</a></p>");
// test("[ga](==)","<p><a href=\"==\">ga</a></p>");
// test("[a](javascript:x)","<p><a href=\"\">a</a></p>");
// test("ga`bu`zo","<p>ga<code>bu</code>zo</p>");
// test("`ga**bu**`zo","<p><code>ga**bu**</code>zo</p>");
// test("ga`**bu**zo`","<p>ga<code>**bu**zo</code></p>");
// test("ga`bu","<p>ga<code>bu</code></p>");
// test("`ga[bu]zo`","<p><code>ga[bu]zo</code></p>");
// test("`ga\nbu","<p><code>ga<br>bu</code></p>");
// test("`ga\n\nbu","<p><code>ga</code></p><p>bu</p>");
// test("```ga","<pre>ga</pre>");
// test("ga```bu","<p>ga</p><pre>bu</pre>");
// test("ga```bu```zo","<p>ga</p><pre>bu</pre><p>zo</p>");
// test("```ga`bu","<pre>ga`bu</pre>");
// test("`ga`bu```zo","<p><code>ga</code>bu</p><pre>zo</pre>");

// echo "\n";

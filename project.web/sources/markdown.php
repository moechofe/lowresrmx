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
 * - ordered list
 * ``` code block
 */

function markdown2html(string $in):String
{
	$in=preg_replace('/\r\n?/',"\n",strip_tags(trim($in)));

	// current parsing state: p, ul, figure, code
	$curr="";
	// current parsing flags:
	$em=false;
	$strong=false;
	$link=false;
	$img=false;
	$newline=false;
	// captured content for links and images
	$captured="";

	$len=strlen($in);

	ob_start();

	$i=-1;
next:
	if(++$i>=$len)goto close;
	$c=mb_substr($in,$i,1);
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
		if(++$i>=$len)goto close;$c=mb_substr($in,$i,1);if($i>=$len)goto end;
		if($c=="\n")goto too_newline;
		goto close;
	}
	else if(($c=="*"||$c=="_") && (!$link || ob_get_level()==2))
	{
		$count=1;
count_stars:
		if(++$i>=$len)goto close;$c=mb_substr($in,$i,1);if($i>=$len)goto end;
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
	else if($c=="!")
	{
		if(++$i>=$len)goto close;$c=mb_substr($in,$i,1);if($i>=$len)goto end;
		if($c=="[")
		{
			$img=true;
			goto close;
		};
		echo "!";
		goto retry;
	}
	else if($c=="[" && !$link)
	{
		if($newline && $curr!="")echo"<br/>";$newline=false;
		if($curr=="" && !$img){$curr="p";echo"<p>";}
		if($img){echo "<figure><img src=\"";$curr="figure";}
		else echo "<a href=\"";
		ob_start();
		$link=true;
		goto next;
	}
	else if($c=="]" && $link)
	{
too_closed:
		if(++$i>=$len)goto close;$c=mb_substr($in,$i,1);if($i>=$len)goto end;
		if($c=="("){$captured=ob_get_clean();goto next;}
		else{echo"]";goto too_closed;}
	}
	else if($c==")" && $link)
	{
		if($img){echo "\"><figcaption>",$captured,"</figcaption></figure>";$img=false;$curr="";}
		else echo "\">",$captured,"</a>";
		$link=false;
		goto next;
	}
	else
	{
		if($newline && $curr!="")
		{
			if($link && ob_get_level()==1) echo rawurlencode("\n");
			else echo"<br/>";
			$newline=false;
		}
		if($curr==""){$curr="p";echo"<p>";}
		if($link && ob_get_level()==1 && in_array($c,[
			// allow characters
			"=","&"])) echo $c;
		elseif($link && ob_get_level()==1 && in_array($c,[
			// allow characters one time only
			"#","?"
		]) && str_contains(ob_get_contents(),$c)===false) echo $c;
		elseif($link && ob_get_level()==1) echo rawurlencode($c);
		else echo htmlspecialchars($c);
		goto next;
	}
	exit;

close:
	if($link && ob_get_level()==2)$captured=ob_get_clean();
	if($link){echo"\">$captured</a>";$link=false;}
	if($em){echo"</em>";$em=false;}
	if($strong){echo"</strong>";$strong=false;}
	// TODO: img and link
	// TODO: capture?
	if($curr){echo"</{$curr}>";$curr="";}
	$newline=false;
	goto retry;

end:
	return ob_get_clean();
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
// test("ga\nbu","<p>ga<br/>bu</p>");
// test("ga\n\nbu","<p>ga</p><p>bu</p>");
// test("ga\n\n\nbu","<p>ga</p><p>bu</p>");
// test("ga *bu* zo","<p>ga <em>bu</em> zo</p>");
// test("ga **bu** zo","<p>ga <strong>bu</strong> zo</p>");
// test("ga ***bu*** zo","<p>ga <strong><em>bu</em></strong> zo</p>");
// test("ga ****bu**** zo","<p>ga <strong><em>bu</em></strong> zo</p>");
// test("ga *bu\nu* zo","<p>ga <em>bu<br/>u</em> zo</p>");
// test("ga **bu\n\nzo** meu","<p>ga <strong>bu</strong></p><p>zo<strong> meu</strong></p>"); // ???? is it possible?
// test("*ga ***bu**** zo*","<p><em>ga </em><strong>bu</strong><em> zo</em></p>");
// test("[ga](bu)","<p><a href=\"bu\">ga</a></p>");
// test("zo\n[ga](bu)","<p>zo<br/><a href=\"bu\">ga</a></p>");
// test("zo[ga]","<p>zo<a href=\"\">ga</a></p>");
// test("zo(ga)","<p>zo(ga)</p>");
// test("[[ga](bu)","<p><a href=\"bu\">[ga</a></p>");
// test("[ga]](bu)","<p><a href=\"bu\">ga]</a></p>");
// test("[ga]((bu)","<p><a href=\"%28bu\">ga</a></p>");
// test("[ga](bu))","<p><a href=\"bu\">ga</a>)</p>");
// test("[*ga*](bu)","<p><a href=\"bu\"><em>ga</em></a></p>");
// test("[ga](*bu*)","<p><a href=\"%2Abu%2A\">ga</a></p>");
// test("*ga[bu](zo)meu*","<p><em>ga<a href=\"zo\">bu</a>meu</em></p>");
// test("ga!bu","<p>ga!bu</p>");
// test("![ga](bu)","<figure><img src=\"bu\"><figcaption>ga</figcaption></figure>");
// test("zo![ga](bu)meu","<p>zo</p><figure><img src=\"bu\"><figcaption>ga</figcaption></figure><p>meu</p>");
// test("![g\na](bu)","<figure><img src=\"bu\"><figcaption>g<br/>a</figcaption></figure>");
// test("![ga](b\nu)","<figure><img src=\"b%0Au\"><figcaption>ga</figcaption></figure>");
// test("![ga](b\tu)","<figure><img src=\"b%09u\"><figcaption>ga</figcaption></figure>");
// test("[ga](##)","<p><a href=\"#%23\">ga</a></p>");
// test("[ga](??)","<p><a href=\"?%3F\">ga</a></p>");
// test("[ga](&&)","<p><a href=\"&&\">ga</a></p>");
// test("[ga](==)","<p><a href=\"==\">ga</a></p>");

// echo "\n";

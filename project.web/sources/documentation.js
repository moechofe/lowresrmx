(()=>new Promise((ready)=>{
	document.addEventListener('readystatechange',ready)
}))().then(async()=>{
<?php
	require_once __DIR__.'/dom.js';
?>

const mediaQuery=window.matchMedia("(max-width: 700px)");

/** @type HTMLElement */
let last_unfold=null;

/** @type function(!HTMLElement,string) */
const fold=(e,child_class)=>{
	let next=e.nextElementSibling;
	while (next.classList.contains(child_class))
	{
		hide(next);
		next=next.nextElementSibling;
	}
};

/** @type function(!HTMLElement,string) */
const unfold=(e,child_class)=>{
	if(mediaQuery.matches) return;
	if(last_unfold) fold(last_unfold,child_class);
	last_unfold=e;
	let next=e.nextElementSibling;
	while (next.classList.contains(child_class))
	{
		show(next);
		next=next.nextElementSibling;
	}
};

queryAll('aside li.h3:has(+ li.h4)').forEach((h3)=>{
	attr(h3,'foldable',true);
	click(h3,()=>unfold(h3,'h4'));
	fold(h3,'h4');
});

const aside=query('aside');

const toogle=()=>{
	if(mediaQuery.matches && !aside.classList.contains('open'))
		aside.classList.add('open');
	else
	{
		aside.classList.remove('open');
	}
};

on(aside,'click',toogle);

const handleScreenChange=(e)=>{
  if(e.matches) {}
	else
	{
		aside.classList.remove('open');
		if(last_unfold) fold(last_unfold,child_class);
  }

	const hash=window.location.hash;
	if(hash)
	{
		const e=query(hash);
		if(e) e.scrollIntoView({behavior:'auto'});
	}
};

on(mediaQuery,'change',handleScreenChange);
handleScreenChange(mediaQuery);
aside.scrollTop=0;

});

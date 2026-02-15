(()=>new Promise((ready)=>{
	document.addEventListener('readystatechange',ready)
}))().then(async()=>{

<?php
	require_once __DIR__.'/config.js';
	require_once __DIR__.'/dom.js';
	require_once __DIR__.'/common.js';
?>

setupSign();

});

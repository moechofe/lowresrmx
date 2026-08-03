(()=>new Promise((ready)=>{
	document.addEventListener('readystatechange',ready)
}))().then(async()=>{

<?php
	require_once __DIR__.'/config.js';
	require_once __DIR__.'/dom.js';
	require_once __DIR__.'/common.js';
	require_once __DIR__.'/list.js';
	require_once __DIR__.'/notification.js';
?>

/** @type {function():Promise<HTMLElement>} */
const setupCommunityList=()=>{
	return get('ranked?w=show').then((ans)=>{
		if(!ans.ok) return Promise.reject("");
		return ans.json();
	}).then((list)=>{
		return setupProgramList(list,{isPost:true});
	});
};

Promise.all([
	setupSign().then(retrieveNotifMaybe),
	setupCommunityList()
]).then((args)=>{
	if(args[1]) injectNotifMarker(args[1]);
}).catch(_=>{});

});

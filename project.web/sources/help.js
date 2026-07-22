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

const post_dialog=setupPostDialog('help');

/** @type {function():Promise<HTMLElement>} */
const setupChatList=()=>{
	return get('latest?w=help').then((ans)=>{
		if(!ans.ok) return Promise.reject("");
		return ans.json();
	}).then((list)=>{
		return setupPostList(list,{isPost:true,isHelp:true});
	});
};

const setupNewTopic=()=>{
	click(query('.new-topic'),(event)=>{
		post_dialog((fid)=>{
			window.location.href=`./${encodeURIComponent(fid)}.html`;
		});
	});
};

Promise.all([
	setupSign().then(retrieveNotifMaybe),
	setupChatList()
]).then((args)=>{
	if(args[1]) injectNotifMarker(args[1]);
});
setupNewTopic();

});

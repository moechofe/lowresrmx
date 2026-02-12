(()=>new Promise((ready)=>{
	document.addEventListener('readystatechange',ready)
}))().then(async()=>{

<?php
	require_once __DIR__.'/dom.js';
	require_once __DIR__.'/common.js';
	require_once __DIR__.'/list.js';
?>

const post_dialog=setupPostDialog('chat');

const setupChatList=()=>{
	get('latest?w=chat').then((ans)=>{
		if(!ans.ok) return Promise.reject("");
		return ans.json();
	}).then((list)=>{
		return setupPostList(list,{isPost:true,isBlog:true});
	}).then(/** @param {!Array<!ProgramItem>} list */(list)=>{
		// console.log(list);
	});
};

const setupNewTopic=()=>{
	click(query('.new-topic'),(_)=>{
		post_dialog((pid)=>{
			window.location.href=`./${encodeURIComponent(pid)}.html`;
		});
	});
};

setupSign();
setupChatList();
setupNewTopic();

});

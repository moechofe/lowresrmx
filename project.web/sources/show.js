(()=>new Promise((ready)=>{
	document.addEventListener('readystatechange',ready)
}))().then(async()=>{

<?php
	require_once __DIR__.'/config.js';
	require_once __DIR__.'/dom.js';
	require_once __DIR__.'/common.js';
	require_once __DIR__.'/list.js';
?>

const setupCommunityList=()=>{
	get('ranked?w=show').then((ans)=>{
		if(!ans.ok) return Promise.reject("");
		return ans.json();
	}).then((list)=>{
		return setupProgramList(list,{isPost:true});
	}).then(/** @param {!Array<!ProgramItem>} list */(list)=>{
		// console.log(list);
	});
};

setupSign();
setupCommunityList();

});

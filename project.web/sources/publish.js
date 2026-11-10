<?php
	require_once __DIR__.'/common.php';
?>
(()=>new Promise(function(ready){
	document.addEventListener('readystatechange',ready)
}))().then(async()=>{

<?php
	require_once __DIR__.'/config.js';
	require_once __DIR__.'/dom.js';
	require_once __DIR__.'/common.js';
?>

const setupCreateReplace=()=>{return new Promise(async(res,rej)=>{
	const createreplace=instanciate(query('#create-replace'));
	const create=instanciate(query('#create'));
	const replace=instanciate(query('#replace'));
	const content=instanciate(query('#content'));
	const button=instanciate(query('#button'));
	const body=query('article');
	const own_choice=find(replace,'.own-choice');
	const own_list=find(replace,'#own-list');
	const own_item=query('#own-item');
	const createbutton=find(createreplace,'button.create');
	const replacebutton=find(createreplace,'button.replace');
	const textarea=find(content,'textarea');
	const limit=find(content,'.limit')
	const title=find(content,'input.title');

	/**
	 * @typedef {{
	 * pid:string,
	 * title:string,
	 * name:string,
	 * }}
	 */
	var OwnItem;

	/** @type integer */
	let cursor=0;

	// /** @type {function(!Array<!OwnItem>):void} */
	const populateList=()=>{
		get('published',{
			[HEADER_TOKEN]:csrf,
			[HEADER_SCAN_CURSOR]:cursor
		}).then((ans)=>{
			if(!ans.ok) return;
			return ans.json();
		}).then((list)=>{
			if(!list) return;
			list.forEach(/** @param {<!OwnItem>} own */(own)=>{
				const item=instanciate(own_item);
				attr(item,'value',`${own.title||"Untitled"} (${own.name||"Unnamed"})`);
				attr(item,'data-eid',own.pid);
				attr(item,'data-title',own.title);
				append(own_list,item);
			});
		});
	};

	/** @type {function():void} */
	const showLimit=()=>{
		const max=dataget(limit,'limit');
		if(!max) return;
		text(limit,`${textarea.value.length}/${max}`);
	};

	hide(create);
	hide(replace);
	hide(content);
	hide(button);

	click(createbutton,()=>{
		show(create);
		hide(replace);
		addClassCond(createbutton,'outline',false);
		addClassCond(replacebutton,'outline',true);
		show(content);
		show(button);
	});

	click(replacebutton,()=>{
		show(replace);
		hide(create);
		addClassCond(createbutton,'outline',true);
		addClassCond(replacebutton,'outline',false);
		show(content);
		show(button);
	});

	input(textarea,(event)=>{
		const max=dataget(limit,'limit');
		if(textarea.value.length>max) textarea.value=textarea.value.substring(0,max);
		showLimit();
		autoHeight(event.target,214);
		event.target.scrollIntoView(true);
	});
	showLimit();
	autoHeight(textarea,214);

	populateList();

	append(body,createreplace);
	append(body,create);
	append(body,replace);
	append(body,content);
	append(body,button);

})};

setupSign().then(user=>{
	setupCreateReplace();
});

});

(()=>new Promise((ready)=>{
	document.addEventListener('readystatechange',ready)
}))().then(async()=>{

<?php require_once __DIR__.'/common.js'; ?>
<?php require_once __DIR__.'/utils.js'; ?>
<?php require_once __DIR__.'/list_first.js'; ?>

	setupSign();

	/** @type {!Array<!FirstItem>} */
	const list_first=await(await get('list_first')).json();

	const list=await setupFirstList(list_first);

// const setupProgramList=()=>{
// 	(async(last_shared,program_item,program_item_nav)=>{
// 		const list=instanciate(last_shared);
// 		const main=query('main');

// 		// Bottom bar

// 		const nav=instanciate(program_item_nav);
// 		click(find(nav,'li:nth-child(1)'),(_)=>{
// 			log("post");
// 		});
// 		click(find(nav,'li:nth-child(2)'),(_)=>{
// 			log("edit");
// 		});
// 		click(find(nav,'li:nth-child(3)'),(_)=>{
// 			log("run");
// 		});
// 		/** @type {function(!Event):void} */
// 		const setupNav=(event)=>{
// 			find(event.target,'nav').append(nav);
// 			visible(nav);
// 		};

// 		/** @type {function(!Event):void} */
// 		const hideNav=(event)=>{
// 			hide(nav);
// 		};

// 		/** @typedef {string} */
// 		var ProgramItem;

// 		// List of programs

// 		/** @type {!Array<!ProgramItem>} */
// 		const my_programs=await(await post('my_programs','')).json();
// 		/** @type {!Array<!HTMLElement>} */
// 		const items=my_programs.map((data)=>{
// 			const item=instanciate(program_item);

// 			find(item,'.name').textContent=data;

// 			// Show the bottom bar
// 			mouseEnter(item,setupNav);
// 			touchStart(item,setupNav);

// 			return item;
// 		});

// 		// List container

// 		list.append.apply(list,items);
// 		main.append(list);

// 		// Hide the bottom bar
// 		mouseLeave(list,hideNav);

// 	})(query('#program-list'),query('#program-item'),query('#program-item-nav'));
// };

// const setupPost=()=>{
// 	(async (post_page,new_post)=>{
// 		const page=instanciate(post_page);
// 		const main=query('main');

// 		const title=find(page,'.title');
// 		const text=find(page,'.text');
// 		const where=find(page,'.where');
// 		const submit=find(page,'.submit');

// 		const isEmpty=(element)=>element.value.trim()==='';

// 		log(title,text,where,submit);

// 		// // Refresh public program list
// 		// const listPublished=async()=>{
// 		// 	log("list published",await(await post('post','test')).body);
// 		// };

// 		// Show the post page

// 		click(new_post,(_)=>{
// 			showAndHideSiblings(page);

// 			// TODO: clean the form
// 			//post_page.hidden=false;
// 			// listPublished();

// 		});

// 		click(submit,(event)=>{
// 			event.preventDefault();

// 			// Check the form
// 			let error=false;
// 			for(const element of [title,text,where])
// 				error=error||isEmpty(element);

// 			// Post the form
// 			if(!error)
// 				post('post',JSON.stringify({
// 					title:title.value,
// 					text:text.value,
// 					where:where.value,
// 				}))

// 			log("post",isEmpty(title),isEmpty(text),isEmpty(where),error);
// 		});

// 		main.append(page);

// 	})(query('#post-page'),query('#new-post'));
// };

// 	setupSign();
// 	setupUploadCard();
// 	setupProgramList();
// 	setupPost();

});

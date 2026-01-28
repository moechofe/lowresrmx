(()=>new Promise((ready)=>{
	document.addEventListener('DOMContentLoaded',ready)
}))().then(async()=>{

<?php
	require_once __DIR__.'/config.js';
	require_once __DIR__.'/dom.js';
	require_once __DIR__.'/common.js';
	require_once __DIR__.'/list.js';
?>

/**
 * @brief Prepare the dialog to publish a program.
 * @return {(pid:string,program_name:string,on_published:(fid:string)=>void)=>void} Function to open the dialog.
 */
const setupPublishDialog=()=>{
	const publish_dialog=query('#publish-dialog');
	const dialog=instanciate(publish_dialog);
	const body=query('body');
	const textarea=find(dialog,'textarea');
	const limit=find(dialog,'.limit')

	/** @type {string|null} */
	let pid=null;

	/** @type {function():void} */
	let cb=null;

	/** @type {function():void} */
	const showLimit=()=>{
		const max=dataget(limit,'limit');
		if(!max) return;
		text(limit,`${textarea.value.length}/${max}`);
	};

	/** @type {function():void} */
	const close=()=>{
		dialogOff(dialog,()=>{
			pid=null;
			cb=null;
			find(dialog,'select.where').value="";
			find(dialog,'input.title').value="";
			textarea.value="";
		});
	};

	click(find(dialog,'button.cancel'),close);

	input(find(dialog,'textarea'),(event)=>{
		const max=dataget(limit,'limit');
		if(textarea.value.length>max) textarea.value=textarea.value.substring(0,max);
		showLimit();
		autoHeight(event.target,214);
		event.target.scrollIntoView(true);
	});
	showLimit();
	autoHeight(find(dialog,'textarea'),214);

	click(find(dialog,'button.publish'),event=>{
		const where=find(dialog,'select.where').value;
		const title=find(dialog,'input.title').value;
		const text=textarea.value;
		if(!where||!title||!text) return;
		if(!pid) return;
		disable(event.target);
		post('/publish',JSON.stringify({
			p:pid,
			w:where,
			i:title,
			x:text,
		}),{
			[HEADER_TOKEN]:csrf
		}).then((res)=>res.json()).then((fid)=>{
			if(cb)cb(fid);
		}).catch((_)=>{
			showError();
		});
	});

	append(body,dialog);

	return (pid_,name,on_published)=>{
		pid=pid_;
		cb=on_published;
		find(dialog,'input.title').value=name||"Untitled";
		dialogOn(dialog);
	};
};

/**
 * @brief Prepare the dialog to delete a program.
 * @return {(pid:string,program_name:string,on_delete:()=>void)=>void} Function to open the dialog.
 */
const setupProgramDelete=()=>{
	const program_delete=query('#program-delete');
	const dialog=instanciate(program_delete);
	const body=query('body');

	/** @type {string|null} */
	let pid=null;

	/** @type {function():void} */
	let cb=null;

	const close=()=>{
		dialogOff(dialog,()=>{
			pid=null;
			cb=null;
			text(find(dialog,'.name'),"");
		});
	};

	click(find(dialog,'button.cancel'),close);

	click(find(dialog,'button.delete'),()=>{
		if(!pid) return;
		post('/delete',JSON.stringify({
			p:pid,
		}),{
			[HEADER_TOKEN]:csrf
		});
		if(cb)cb();
		close();
	});

	append(body,dialog);

	return (pid_,name,on_delete)=>{
		pid=pid_;
		cb=on_delete;
		text(find(dialog,'.name'),name||"Untitled");
		dialogOn(dialog);
	};
};

/**
 * @brief Prepare the dialog to replace a program of a topic.
 * @return {(pid:string,program_name:string,on_replaced:(fid:string)=>void)=>void} Function to open the dialog.
 */
const setupReplaceDialog=()=>{
	const replace_dialog=query('#replace-dialog');
	const own_item=query('#own-item');
	const dialog=instanciate(replace_dialog);
	const body=query('body');
	const own_list=find(dialog,'select.own-list');
	const publish=find(dialog,'button.publish');
	const textarea=find(dialog,'textarea');
	const limit=find(dialog,'.limit');
	const title=find(dialog,'input.title');

	/**
	 * @typedef {{
	 * pid:string,
	 * title:string,
	 * name:string,
	 * }}
	 */
	var OwnItem;

	/** @type {string|null} */
	let pid=null;

	/** @type {function():void} */
	let cb=null;

	/** @type integer */
	let cursor=0;

	// /** @type {function(!Array<!OwnItem>):void} */
	const populateList=()=>{
		get('own',{
			[HEADER_TOKEN]:csrf,
			[HEADER_SCAN_CURSOR]:cursor
		}).then((ans)=>{
			if(!ans.ok) return;
			return ans.json();
		}).then((list)=>{
			if(!list) return;
			list.forEach(/** @param {<!OwnItem>} own */(own)=>{
				const item=instanciate(own_item);
				text(item,`${own.title||"Untitled"} (${own.name||"Unnamed"})`);
				attr(item,'data-eid',own.pid);
				attr(item,'data-title',own.title);
				append(own_list,item);
			});
		});
	};

	on(own_list,'change',(evt)=>{
		const option=evt.target.selectedOptions[0]
		loading(true);
		title.value=option.dataset['title'];
		delay(500,_=>{
			get(`own?id=${encodeURI(option.dataset['eid'])}`,{
				[HEADER_TOKEN]:csrf,
			}).then((ans)=>{
				if(!ans.ok) return Promise.reject();
				return ans.json();
			}).then((data)=>{
				textarea.value=data||"";
				showLimit();
			}).finally(()=>{
				loading(false);
			});
		});
	});

	/** @type {function():void} */
	const showLimit=()=>{
		const max=dataget(limit,'limit');
		if(!max) return;
		text(limit,`${textarea.value.length}/${max}`);
	};

	/** @type {function():void} */
	const close=()=>{
		dialogOff(dialog,()=>{
			pid=null;
			cb=null;
			find(dialog,'select.own-list').selectedIndex=0;
			findAll(own_list,'.own-item').forEach((opt)=>opt.remove());
			title.value="";
			textarea.value="";
			loading(false);
		});
	};

	/** @type {function(yes:boolean):void} */
	const loading=(yes)=>{
		if(yes) {
			busy(publish);
			disable(publish);
			disable(own_list);
			disable(textarea);
		}
		else
		{
			unbusy(publish);
			enable(publish);
			enable(own_list);
			enable(textarea);
		}
	};

	click(find(dialog,'button.cancel'),close);

	input(textarea,(event)=>{
		const max=dataget(limit,'limit');
		if(textarea.value.length>max) textarea.value=textarea.value.substring(0,max);
		showLimit();
		autoHeight(event.target,214);
		event.target.scrollIntoView(true);
	});
	showLimit();
	autoHeight(textarea,214);

	click(find(dialog,'button.publish'),event=>{
		const new_title=title.value.trim();
		const new_text=textarea.value.trim();
		if(!new_title||!new_text) return;
	// 	if(!where||!title||!text) return;
	// 	if(!pid) return;
	// 	disable(event.target);
	// 	post('/publish',JSON.stringify({
	// 		p:pid,
	// 		w:where,
	// 		i:title,
	// 		x:text,
	// 	})).then((res)=>res.json()).then((fid)=>{
	// 		if(cb)cb(fid);
	// 	}).catch((_)=>{
	// 		showError();
	// 	});
	});

	append(body,dialog);

	return (pid_,name,on_replaced)=>{
		pid=pid_;
		cb=on_replaced;
		// find(dialog,'input.title').value=name||"Untitled";
		dialogOn(dialog);
		populateList();
	};
};

const delete_dialog=setupProgramDelete();
const publish_dialog=setupPublishDialog();
const replace_dialog=setupReplaceDialog();

const setupShareList=()=>{
	get('last_shared').then((ans)=>{
		if(!ans.ok) return Promise.reject("");
		return ans.json();
	}).then((list)=>{
		return setupProgramList(list,{isShare:true});
	}).then(/** @param {!Array<!ProgramItem>} list */(list)=>{
		on(list,'ask_to_delete',(event)=>{
			delete_dialog(event.detail.pid,event.detail.name,()=>{
				emit(list,'remove_item',{pid:event.detail.pid});
			});
		});

		on(list,'ask_to_publish',(event)=>{
			publish_dialog(event.detail.pid,event.detail.name,(pid)=>{
				window.location.href=`./${encodeURIComponent(pid)}.html`;
			});
		});

		on(list,'ask_to_replace',(event)=>{
			replace_dialog(event.detail.pid,event.detail.name,(pid)=>{
				window.location.href=`./${encodeURIComponent(pid)}.html`;
			});
		});
	})
	.catch((_)=>{});
};

setupSign().then(user=>{
	if(!user) window.location.href='/community.html';
});
setupError();
setupShareList();

});

/**
 * @typedef {{
 * pid: string,
 * name: string?,
 * title: string?
 * author: string,
 * ut: string?
 * ct: string?,
 * }}
 */
var ProgramItem;

/**
 * @typedef {{
 * isShare: boolean,
 * isPost: boolean,
 * }}
 */
var ProgramListConfig;

/**
 * @brief List programs and present them with possible interactions.
 * @param {!Array<!ProgramItem>} prg_list List of programs.
 * @param {ProgramListConfig} config Configuration of the list.
 * @return {Promise<HTMLElement>} The list of programs.
 * @event ask_to_publish {pid:string,name:string} When the user wants to publish a program.
 * @event ask_to_delete {pid:string,name:string} When the user wants to delete a program.
 */
const setupProgramList=(prg_list,config)=>{return new Promise(async(res,rej)=>{
	const item_tpl=query('#program-item');
	const list=query('.program-list');
	const body=query('body');

	/** @type {!Array<!HTMLElement>} */
	const items=prg_list.map(data=>{
		const item=instanciate(item_tpl);
		const a=find(item,'a');

		dataset(item,'pid',data.pid);

		find(item,'.name').textContent=data.title||data.name||"Untitled";
		find(item,'.picture').style.backgroundImage="url(\"./"+data.pid+".png\")";
		find(item,'.author').textContent=data.author||"Unknown";
		humanDate(find(item,'.date'),data.ut||data.ct);

		if(config.isPost)	find(item,'.points').textContent=data.points||"?";
		else hide(find(item,'.points'));

		if(config.isShare) show(find(item,'details'));

		if(config.isPost) attr(a,"href",`./${encodeURI(data.pid)}.html`);
		addClassCond(a,"is-post",config.isPost);

		dataset(find(item,'.publish'),'pid',data.pid);

		click(find(item,'.publish'),_=>{
			emit(list,'ask_to_publish',{
				pid:data.pid,
				name:data.name,
			});
		});

		click(find(item,'.delete'),_=>{
			emit(list,'ask_to_delete',{
				pid:data.pid,
				name:data.name,
			});
		});

		return item;
	});

	on(list,'remove_item',(event)=>{
		log("remove_item",event.detail);
		find(list,'.program-item[data-pid="'+event.detail.pid+'"]').remove();
	});

	// List container
	list.append.apply(list,items);

	res(list);
})};


/**
 * @brief List programs and present them with possible interactions.
 * @param {!Array<!ProgramItem>} prg_list List of programs.
 * @param {ProgramListConfig} config Configuration of the list.
 * @return {Promise<HTMLElement>} The list of programs.
 * @event ask_to_publish {pid:string,name:string} When the user wants to publish a program.
 * @event ask_to_delete {pid:string,name:string} When the user wants to delete a program.
 */
const setupPostList=(prg_list,config)=>{return new Promise(async(res,rej)=>{
	const item_tpl=query('#post-item');
	const list=query('.post-list');
	const body=query('body');

	/** @type {!Array<!HTMLElement>} */
	const items=prg_list.map(data=>{
		const item=instanciate(item_tpl);
		const a=find(item,'a');

		dataset(item,'pid',data.pid);

		find(item,'.name').textContent=data.title||data.name||"Untitled";
		find(item,'.picture').style.backgroundImage="url(\"./"+data.pid+".png\")";
		find(item,'.author').textContent=data.author||"Unknown";
		humanDate(find(item,'.date'),data.ut||data.ct);
		find(item,'.points').textContent=data.points||"?";

		if(config.isShare) show(find(item,'details'));

		if(config.isPost) attr(a,"href",`./${encodeURI(data.pid)}.html`);
		addClassCond(a,"is-post",config.isPost);

		return item;
	});

	on(list,'remove_item',(event)=>{
		find(list,'.program-item[data-pid="'+event.detail.pid+'"]').remove();
	});

	// List container
	list.append.apply(list,items);

	res(list);
})};


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
		}),null);
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
 * @brief Prepare the dialog to publish a program.
 * @return {(pid:string,program_name:string,on_published:(fid:string)=>void)=>void} Function to open the dialog.
 */
const setupPublishDialog=()=>{
	const publish_dialog=query('#publish-dialog');
	const dialog=instanciate(publish_dialog);
	const body=query('body');

	/** @type {string|null} */
	let pid=null;

	/** @type {function():void} */
	let cb=null;

	const showLimit=()=>{
		const limit=find(dialog,'.limit');
		const ta=find(dialog,'textarea');
		const max=dataget(limit,'limit');
		if(!max) return;
		limit.textContent=`${ta.value.length}/${max}`;
	};

	const close=function(){
		dialogOff(dialog,()=>{
			pid=null;
			cb=null;
			find(dialog,'select.where').value="";
			find(dialog,'input.title').value="";
			find(dialog,'textarea.text').value="";
		});
	};

	click(find(dialog,'button.cancel'),close);

	input(find(dialog,'textarea'),(event)=>{
		const max=dataget(find(dialog,'.limit'),'limit');
		const ta=find(dialog,'textarea');
		if(ta.value.length>max) ta.value=ta.value.substring(0,max);
		showLimit();
		autoHeight(event.target,214);
		event.target.scrollIntoView(true);
	});
	showLimit();
	autoHeight(find(dialog,'textarea'),214);

	click(find(dialog,'button.publish'),event=>{
		const where=find(dialog,'select.where').value;
		const title=find(dialog,'input.title').value;
		const text=find(dialog,'textarea.text').value;
		if(!where||!title||!text) return;
		if(!pid) return;
		disable(event.target);
		post('/publish',JSON.stringify({
			p:pid,
			w:where,
			i:title,
			x:text,
		})).then((res)=>res.json()).then((fid)=>{
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
 * @brief Prepare the dialog to post a topic.
 * @param {string} where
 * @return {(on_posted:(fid:string)=>void)=>void} Function to open the dialog.
 */
const setupPostDialog=(where)=>{
	const post_dialog=query('#post-dialog');
	const dialog=instanciate(post_dialog);
	const body=query('body');
	const content=find(dialog,'.content');

	/** @type {function():void} */
	let cb=null;

	const close=function(){
		dialogOff(dialog,()=>{
			cb=null;
			find(dialog,'select.where').value="";
			find(dialog,'input.title').value="";
			find(dialog,'textarea.text').value="";
		});
	};

	const showLimit=()=>{
		const limit=find(content,'.limit');
		const ta=find(content,'textarea');
		const max=dataget(limit,'limit');
		if(!max) return;
		limit.textContent=`${ta.value.length}/${max}`;
	};

	input(find(content,'textarea'),(event)=>{
		const max=dataget(find(content,'.limit'),'limit');
		const ta=find(content,'textarea');
		if(ta.value.length>max) ta.value=ta.value.substring(0,max);
		showLimit();
		autoHeight(event.target,214);
		event.target.scrollIntoView(true);
	});
	showLimit();
	autoHeight(find(content,'textarea'),214);

	click(find(dialog,'button.cancel'),close);

	click(find(dialog,'button.publish'),event=>{
		const where=find(dialog,'select.where').value;
		const title=find(dialog,'input.title').value;
		const text=find(dialog,'textarea.text').value;
		if(!where||!title||!text) return;
		disable(event.target);
		post('/post',JSON.stringify({
			w:where,
			i:title,
			x:text,
		})).then((res)=>res.json()).then((fid)=>{
			if(cb)cb(fid);
		}).catch((_)=>{
			showError();
		}).finally((_)=>{
			enable(event.target);
		});
	});

	append(body,dialog);

	return (on_posted)=>{
		cb=on_posted;
		find(dialog,'select.where').value=where;
		dialogOn(dialog);
	};
};

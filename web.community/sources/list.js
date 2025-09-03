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
	const list=instanciate(query('#program-list'));
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

		// // TODO: find a way to open the App with the program
		// if(/Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent))
		// 	find(item,'.play').remove();

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
	body.append(list);

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
	const list=instanciate(query('#post-list'));
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

		// // TODO: find a way to open the App with the program
		// if(/Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent))
		// 	find(item,'.play').remove();

		if(config.isShare) show(find(item,'details'));

		if(config.isPost) attr(a,"href",`./${encodeURI(data.pid)}.html`);
		addClassCond(a,"is-post",config.isPost);

		// dataset(find(item,'.publish'),'pid',data.pid);

		// click(find(item,'.publish'),_=>{
		// 	emit(list,'ask_to_publish',{
		// 		pid:data.pid,
		// 		name:data.name,
		// 	});
		// });

		// click(find(item,'.delete'),_=>{
		// 	emit(list,'ask_to_delete',{
		// 		pid:data.pid,
		// 		name:data.name,
		// 	});
		// });

		return item;
	});

	on(list,'remove_item',(event)=>{
		log("remove_item",event.detail);
		find(list,'.program-item[data-pid="'+event.detail.pid+'"]').remove();
	});

	// List container
	list.append.apply(list,items);
	body.append(list);

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

	body.append(dialog);

	return (pid_,name,on_delete)=>{
		pid=pid_;
		cb=on_delete;
		text(find(dialog,'.name'),name||"Untitled");
		dialogOn(dialog);
	};
};

/**
 * @brief Prepare the dialog to publish a program.
 * @return {(pid:string,program_name:string,on_published:()=>void)=>void} Function to open the dialog.
 */
const setupPublishDialog=()=>{
	const publish_dialog=query('#publish-dialog');
	const dialog=instanciate(publish_dialog);
	const body=query('body');

	/** @type {string|null} */
	let pid=null;

	/** @type {function():void} */
	let cb=null;

	const close=function(){
		dialogOff(dialog,()=>{
			pid=null;
			cb=null;
			// TODO: clean the fields
			find(dialog,'select#where').value="";
			find(dialog,'input#title').value="";
			find(dialog,'textarea#text').value="";
		});
	};

	click(find(dialog,'button.cancel'),close);

	click(find(dialog,'button.publish'),event=>{
		disable(event.target);
		const where=find(dialog,'select#where').value;
		const title=find(dialog,'input#title').value;
		const text=find(dialog,'textarea#text').value;
		if(!where||!title||!text) return;
		if(!pid) return;
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

	// Cancel button from the dialog

	body.append(dialog);

	return (pid_,name,on_published)=>{
		pid=pid_;
		cb=on_published;
		find(dialog,'input#title').value=name||"Untitled";
		dialogOn(dialog);
	};
};

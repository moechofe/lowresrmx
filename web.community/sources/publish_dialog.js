const setupPublishDialog=(program_list)=>{return new Promise(async function(res,rej){
	const publish_dialog=query('#publish-dialog');
	const dialog=instanciate(publish_dialog);
	const body=query('body');
	const html=query('html');

	/** @type {function(string):void} */
	const open=function(){
		addClassCond(html,'modal-is-open',true);
		addClassCond(html,'modal-is-opening',true);
		addAttrCond(dialog,'open','',true);
		delay(200,_=>{
			addClassCond(html,'modal-is-opening',false);
		});
	};

	const close=function(){
		addClassCond(html,'modal-is-open',false);
		addClassCond(html,'modal-is-closing',true);
		delay(200,_=>{
			addAttrCond(dialog,'open','',false);
			addClassCond(html,'modal-is-closing',false);
		});
	};

	// Publish button for the list

	/** @type {string|null} */
	let pid=null;

	findAll(program_list,'button.publish').forEach((publish)=>{
		click(publish,(event)=>{
			pid=dataget(event.target,'pid');
			open();
		});
	});

	// Publish button for the dialog

	click(find(dialog,'button.publish'),(event)=>{
		const where=find(dialog,'select#where').value;
		const title=find(dialog,'input#title').value;
		const text=find(dialog,'textarea#text').value;
		log(where,title,text);
		if(!where||!title||!text) return;
		if(!pid) return;
		post('/publish',JSON.stringify({
			p:pid,
			w:where,
			i:title,
			x:text,
		})).then((res)=>res.json()).then((data)=>{
			log(data);
		});
		// TODO: post the data and gather entry id
		// TODO: relocate to the post
	});

	// Cancel button

	click(find(dialog,'button.cancel'),()=>{
		close();
	});

	body.append(dialog);

	res();
});};

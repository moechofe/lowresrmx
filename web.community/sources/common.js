const HEADER_TOKEN='X-Application-Token';
const HEADER_FILE_TYPE='X-Application-Type';
const HEADER_SESSION='X-Application-Session';

const doc=document;
const log=console.log.bind(console);

/** @type {function(string):!HTMLElement} */
const create=(tag)=>doc.createElement(tag);

/** @type {function(string):!HTMLElement} */
const query=(query)=>doc.querySelector(query)||create('div');

/** @type {function(string):!NodeListOf<!HTMLElement>} */
const queryAll=(query)=>doc.querySelectorAll(query);

/** @type {function(string):!HTMLElement} */
const find=(elem,query)=>elem.querySelector(query)||create('div');

/** @type {function(string):!Array<!HTMLElement>} */
const findAll=(elem,query)=>elem.querySelectorAll(query);

/** @type {function(string):!HTMLElement} */
const instanciate=(tpl)=>tpl.content.cloneNode(true).firstElementChild;

/** @type {function(!HTMLElement,string,any):void} */
const emit=(element,event_name,detail)=>{element.dispatchEvent(new CustomEvent(event_name,{detail:detail}))};

/** @type {function(!HTMLElement,string,function(!Event):void):void} */
const on=(element,event_name,callback)=>{element.addEventListener(event_name,callback)};

/** @type {function(!HTMLElement,function(!Event):void):void} */
const click=(element,callback)=>{element.addEventListener('click',callback)};

/** @type {function(!HTMLElement,function(!Event):void):void} */
const touchStart=(element,callback)=>{element.addEventListener('touchstart',callback)};

/** @type {function(!HTMLElement,function(!Event):void):void} */
const mouseEnter=(element,callback)=>{element.addEventListener('mouseenter',callback)};

/** @type {function(!HTMLElement,function(!Event):void):void} */
const mouseLeave=(element,callback)=>{element.addEventListener('mouseleave',callback)};

/** @type {function(!HTMLElement,function(!Event):void):void} */
const mouseOut=(element,callback)=>{element.addEventListener('mouseout',callback)};

/** @type {function(!HTMLElement):void} */
const hide=(element)=>element.hidden=true;

/** @type {function(!HTMLElement):void} */
const show=(element)=>element.hidden=false;

/** @type {function(!HTMLElement):void} */
const showAndHideSiblings=(element)=>{
	for(const sibling of element.parentElement.children)
		if(sibling!=element) sibling.hidden=true;
		else sibling.hidden=false;
}

/** @type {function(!HTMLElement):void} */
const visible=(element)=>{element.style.visibility='visible';}

/** @type {function(!HTMLElement,string,bool):void} */
const addClassCond=(element,className,condition)=>{
	if(condition) element.classList.add(className);
	else element.classList.remove(className);
}

/** @type {function(!HTMLElement,string):void} */
const addAttrCond=(element,name,value,condituon)=>{
	if(condituon) element.setAttribute(name,value);
	else element.removeAttribute(name);
}

/** @type {function(!HTMLElement,string,any):void} */
const dataset=(element,key,val)=>element.dataset[key]=val;

/** @type {function(!HTMLElement,string):any} */
const dataget=(element,key)=>element.dataset[key];

/** @type {function(!HTMLElement,string):void} */
const text=(element,text)=>{element.innerText=text};

/** @type {function(!HTMLElement,function():void):void} */
const dialogOn=(dialog,done)=>{
	const html=query('html');
	addClassCond(html,'modal-is-open',true);
	addClassCond(html,'modal-is-closing',false);
	addClassCond(html,'modal-is-opening',true);
	addAttrCond(dialog,'open','',true);
	delay(800,_=>{
		addClassCond(html,'modal-is-opening',false);
		if(done)done();
	});
};

const dialogOff=(dialog,done)=>{
	const html=query('html');
	addClassCond(html,'modal-is-opening',false);
	addClassCond(html,'modal-is-closing',true);
	delay(800,_=>{
		addAttrCond(dialog,'open','',false);
		addClassCond(html,'modal-is-open',false);
		addClassCond(html,'modal-is-closing',false);
		if(done)done();
	});
}


/** @type {function(number,function):void} */
const delay=(ms,func)=>setTimeout(func,ms);

/** @type {function(string,string,Map<String,Sring>):!Promise<!Response>} */
const post=(path,body,headers)=>fetch(path,{
	method:'POST',
	mode:'cors',
	cache:'no-cache',
	credentials:'same-origin',
	headers:Object.assign({
		'Content-Type':'application/x-binary',
		[HEADER_SESSION]:window.localStorage.getItem('session')||'',
	},headers),
	redirect:'follow',
	referrerPolicy:'no-referrer',
	body:body,
});

/** @type {function(string,Map<String,String>):!Promise<!Response>} */
const get=(path,headers)=>fetch(path,{
	method:'GET',
	mode:'cors',
	cache:'no-cache',
	credentials:'same-origin',
	headers:Object.assign({
		[HEADER_SESSION]:window.localStorage.getItem('session')||'',
	},headers),
	redirect:'follow',
	referrerPolicy:'no-referrer',
});

/** @type {function(!File,String):!Promise<!Response>} */
const upload=(type,file,token)=>{
	const reader=new FileReader();
	const xhr=new XMLHttpRequest();
	return new Promise((res,rej)=>{
		reader.onload=(event)=>{
			post('upload',event.target.result,{
				[HEADER_FILE_TYPE]: type,
				[HEADER_TOKEN]: token,
			}).then(res).catch(rej);
		};
		reader.error=rej;
		reader.readAsBinaryString(file);
	});
};

/** @type {function():void} */
// TODO: deleteme, it should only be available throughout the iOS app
const setupUploadCard=()=>{
	(async (upload_card,share_program)=>{

		const inputs=findAll(upload_card,'input');
		const blocker=find(upload_card,'.blocker');
		const submit=find(upload_card,'.submit');
		const cancel=find(upload_card,'.cancel');

		const close=()=>{
			upload_card.hidden=true;
			blocker.hidden=false;
		}

		/** @type {string} */
		let token="";

		// wait for the card to be openned
		click(share_program,async(_)=>{
			// close the card
			if(upload_card.hidden==false) return close();

			// open the card, block the share button until the token is received
			share_program.ariaDisabled=true;
			blocker.hidden=false;
			upload_card.hidden=false;

			// wait for the token
			token=(await get('token','')).headers.get(HEADER_TOKEN);
			if(token)
			{
				blocker.hidden=true;
				share_program.ariaDisabled=false;
			}

			// automaticly close the card after 1 hour, the token will be invalid
			setTimeout(close,60*60*1000);
		});

		// upload the program and the image
		click(submit,async(event)=>{
			event.preventDefault();
			blocker.hidden=false;
			const prg=inputs[0].files[0]?upload('prg',inputs[0].files[0],token):Promise.reject();
			const img=inputs[1].files[0]?upload('img',inputs[1].files[0],token):Promise.resolve();
			await Promise.all([prg,img]);
			await post('publish','',{
				[HEADER_TOKEN]: token,
			});
			log("shared");
		});

		// to close the card
		click(cancel,(event)=>{
			event.preventDefault();
			close();
		});
	})(query('#upload-card'),query('#share-program'));
};

// TODO: move to header.js
const setupSign=()=>{
	post('is_signed','')
	.then((res)=>res.json())
	.then((signed)=>{
		log("signed",signed);
		queryAll(signed===false?'.is-not-signed':'.is-signed').forEach((item)=>show(item));
		if(signed)
		{
			text(query('.user-profile .name'),signed.author?signed.author:"no author name yet");
			// query('.user-profile .picture').style.backgroundImage="url('"+signed.picture+"')";
		}
	}).catch((_)=>queryAll('.is-not-signed').forEach((item)=>show(item)));

	((google_sign_in)=>{
		click(google_sign_in,(_)=>{
			window.location.href='/google';
		});
	})(query('button.google-sign-in'));

	((discord_sign_in)=>{
		click(discord_sign_in,(_)=>{
			window.location.href='/discord';
		});
	})(query('button.discord-sign-in'));
};

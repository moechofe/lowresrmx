const HEADER_TOKEN='X-Application-Token';
const HEADER_FILE_TYPE='X-Application-Type';
const HEADER_SESSION='X-Application-Session';

const doc=document;
const log=console.log.bind(console);

/** @type {function():!Promise} */
const domReady=()=>new Promise(function(ready){
	doc.addEventListener('readystatechange',ready)
});

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
const block=(element)=>{element.style.display='block';}

/** @type {function(!HTMLElement):void} */
const flex=(element)=>{element.style.display='flex';}

/** @type {function(!HTMLElement):void} */
const none=(element)=>{element.style.display='none';}

/** @type {function(!HTMLElement):bool} */
const isDisplayed=(element)=>element.style.display!=='none';

/** @type {function(!HTMLElement):void} */
const hidden=(element)=>{element.style.visibility='hidden';}

/** @type {function(!HTMLElement):void} */
const visible=(element)=>{element.style.visibility='visible';}

/** @type {function(string,string,Map<String,Sring>):!Promise<!Response>*/
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
const setupUploadCard=()=>{
	(async (upload_card,share_program)=>{
		const inputs=findAll(upload_card,'input');
		const blocker=find(upload_card,'.blocker');
		const submit=find(upload_card,'button');

		/** @type {string} */
		let token="";

		// wait for the card to be openned
		click(share_program,async(event)=>{
			share_program.ariaDisabled=true;
			blocker.hidden=false;
			upload_card.hidden=false;

			// wait for the token
			token=(await post('token','')).headers.get(HEADER_TOKEN);
			if(token)
			{
				blocker.hidden=true;
				share_program.ariaDisabled=false;
			}
		});

		// automaticly close the card after 1 hour
		setTimeout(()=>{
			upload_card.hidden=true;
			blocker.hidden=false;
		},60*60*1000);

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
	})(query('#upload-card'),query('#share-program'));
};

const setupProgramList=()=>{
	(async(program_list,program_item,program_item_nav)=>{
		const list=instanciate(program_list);
		const main=query('main');

		// Bottom bar

		const nav=instanciate(program_item_nav);
		click(find(nav,'li:nth-child(1)'),(event)=>{
			log("post");
		});
		click(find(nav,'li:nth-child(2)'),(event)=>{
			log("edit");
		});
		click(find(nav,'li:nth-child(3)'),(event)=>{
			log("run");
		});
		/** @type {function(!Event):void} */
		const setupNav=(event)=>{
			find(event.target,'nav').append(nav);
			visible(nav);
		};

		/** @type {function(!Event):void} */
		const hideNav=(event)=>{
			hidden(nav);
		};

		/** @typedef {string} */
		var ProgramItem;

		// List of programs

		/** @type {!Array<!ProgramItem>} */
		const my_programs=await(await post('my_programs','')).json();
		/** @type {!Array<!HTMLElement>} */
		const items=my_programs.map((data)=>{
			const item=instanciate(program_item);

			find(item,'.name').textContent=data;

			// Show the bottom bar
			mouseEnter(item,setupNav);
			touchStart(item,setupNav);

			return item;
		});

		// List container

		list.append.apply(list,items);
		main.append(list);

		// Hide the bottom bar
		mouseLeave(list,hideNav);

	})(query('#program-list'),query('#program-item'),query('#program-item-nav'));
};

const setupSign=()=>{
	post('is_signed','')
	.then((res)=>res.json())
	.then((signed)=>{
		queryAll(signed?'.is-signed':'.is-not-signed').forEach((item)=>item.hidden=false);
	});

	((google_sign_in)=>{
		click(google_sign_in,(event)=>{
			window.location.href='/google';
		});
	})(query('#google-sign-in'))
};

domReady().then(()=>{

	setupUploadCard();
	setupProgramList();
	setupSign();

});

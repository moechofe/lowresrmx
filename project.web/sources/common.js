const HEADER_TOKEN='X-Application-Token';
const HEADER_FILE_TYPE='X-Application-Type';
const HEADER_SESSION='X-Application-Session';
const HEADER_SCAN_CURSOR='X-Application-Cursor';


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

/** @type {function(!HTMLElement,!HTMLElement)} */
const append=(parent,element)=>parent.append(element);

/** @type {function(!HTMLElement)} */
const clear=(parent)=>parent.innerHTML='';

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

/** @type {function(!HTMLElement,function(!Event):void):void} */
const input=(element,callback)=>{element.addEventListener('input',callback)};

/** @type {function(!HTMLElement):void} */
const hide=(element)=>element.hidden=true;

/** @type {function(!HTMLElement):void} */
const show=(element)=>element.hidden=false;

/** @type {function(!HTMLElement,!string):void } */
const hideAll=(element,query)=>element.querySelectorAll(query).forEach((item)=>hide(item));

/** @type {function(!HTMLElement,!string):void } */
const showAll=(element,query)=>element.querySelectorAll(query).forEach((item)=>show(item));

/** @type {function(!HTMLElement):void} */
const attr=(element,name,value)=>{
	if(value===null) return element.removeAttribute(name);
	element.setAttribute(name,value);
}

/** @type {function(!HTMLElement):void} */
const disable=(element)=>element.setAttribute('disabled','');

/** @type {function(!HTMLElement):void} */
const enable=(element)=>element.removeAttribute('disabled');

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
	const old=document.querySelector('dialog[open]');
	if(old)
	{
		dialogOff(old);
		delay(100,_=>{
			dialogOn(dialog,done);
		});
		return;
	}
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

/** @type {function(!HTMLElement):void} */
const autoHeight=(element,initial)=>{
	element.style.height='';
	element.style.height=Math.max(initial,element.scrollHeight)+'px';
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
		// [HEADER_SESSION]:window.localStorage.getItem('session')||'',
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
		// [HEADER_SESSION]:window.localStorage.getItem('session')||'',
	},headers),
	redirect:'follow',
	referrerPolicy:'no-referrer',
});

/** @type {function(!File,String):!Promise<!Response>} */
const upload=(type,file,token)=>{
	const reader=new FileReader();
	return new Promise((res,rej)=>{
		reader.onload=(event)=>{
			post('upload',event.target.result,{
				[HEADER_FILE_TYPE]: type,
				[HEADER_TOKEN]: token,
			}).then(res).catch(rej);
		};
		reader.error=rej;
		reader.readAsArrayBuffer(file);
	});
};

const setupMobile=()=>{
	if(window.matchMedia("(any-hover:none)").matches)
		queryAll('.is-mobile').forEach((item)=>show(item));
}

/** @type {function(!HTMLElement,string):void} */
const humanDate=(elem,timestamp)=>{
	/** @type {!Date} */
	var date=null;
	if(timestamp)
	{
		// Date in ISO 8601 format
		const epoch=Date.parse(timestamp);
		if(epoch) date=new Date(epoch);
	}
	// TODO: get the locale from the user throught cookie or sessions
	text(elem,date?date.toLocaleDateString('en-US',{
		year:'numeric',
		month:'short',
		day:'numeric'
	}):"Unknown date");
};


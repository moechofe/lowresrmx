
/** @type {function(string,string,Map<String,Sring>):!Promise<!Response>} */
const post=(path,body,headers)=>fetch(path,{
	method:'POST',
	mode:'cors',
	cache:'no-cache',
	credentials:'same-origin',
	headers:Object.assign({
		'Content-Type':'application/x-binary',
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

/** @type {function():void} */
const setupMobile=()=>{
	const ua=navigator.userAgent;
	const is_ios=/iPad|iPhone|iPod/i.test(ua);
	const is_mobile=is_ios||/Android|webOS|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(ua)||window.matchMedia("(any-hover:none)").matches;

	if(is_mobile)
		queryAll('.is-mobile').forEach((item)=>show(item));
	else
		queryAll('.is-not-mobile').forEach((item)=>show(item));

	if(is_ios)
		queryAll('.is-ios').forEach((item)=>show(item));
	else
		queryAll('.is-not-ios').forEach((item)=>show(item));
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
	if (date && elem.tagName.toLowerCase() === 'time') {
		attr(elem, 'datetime', date.toISOString());
	}
	// TODO: get the locale from the user throught cookie or sessions
	text(elem,date?date.toLocaleDateString('en-US',{
		year:'numeric',
		month:'short',
		day:'numeric'
	}):"Unknown date");
};
/** @type {function():void} */
const setupError=()=>{
	const error=query('#error');
	const dialog=instanciate(error);
	query('body').append(dialog);
}

/** @type {function():void} */
const showError=()=>{
	dialogOn(query('dialog.error'));
}

/** @typedef {{
 * author: string,
 * picture: string,
 * token: string
 * }}
 */
var IsSigned;

/** @type string */
var csrf;

/** @type {function():Promise<IsSigned>} */
const setupSign=()=>new Promise(res=>{
	post('is_signed','')
	.then((res)=>res.json())
	.then((/** @type !IsSigned|false */signed)=>{
		if(signed!==false)
		{
			signed=/** @type !IsSigned */(signed);
			text(query('.user-profile .name'),signed.author?signed.author:"No author name yet");
			//query('.user-profile .picture').style.backgroundImage="url('"+signed.picture+"')";
			csrf=signed.token;
			queryAll('.is-signed').forEach((item)=>show(item));
			res(signed);
		}
		else
		{
			queryAll('.is-not-signed').forEach((item)=>show(item));
			res(null);
		}
	}).catch(_=>{
		queryAll('.is-not-signed').forEach((item)=>show(item));
		res(null);
	}).finally((_)=>queryAll('.is-loading').forEach((item)=>hide(item)));

	click(query('button.google-sign-in'),(_)=>{
		window.location.href='/google';
	});

	click(query('button.discord-sign-in'),(_)=>{
		window.location.href='/discord';
	});

	click(query('button.github-sign-in'),(_)=>{
		window.location.href='/github';
	});

	click(query('.sign-out a'),(_)=>{
		post('sign_out','',{
			[HEADER_TOKEN]:csrf
		})
		.then((res)=>res.json())
		.then((signed_out)=>{
			window.location.href='/community.html';
		});
	});
});

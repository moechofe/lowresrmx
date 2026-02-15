(()=>new Promise((ready)=>{
	document.addEventListener('readystatechange',ready)
}))().then(async()=>{

<?php
	require_once __DIR__.'/config.js';
	require_once __DIR__.'/dom.js';
	require_once __DIR__.'/common.js';
?>

/** @type {function(!IsSigned):void} */
const setupSetting=(user)=>{
	const input=query('fieldset input.author');
	const label=query('fieldset label.author');

	if(user.author) attr(input,'value',user.author);
	else attr(input,'placeholder',"Enter an author name");

	on(input,'change',()=>{
		if(input.value.length<4) return;
		console.log("here",input.value);
		disable(input);
		busy(label);
		delay(200,_=>{
			post('author',input.value.substring(0,100),{
				'Content-Type':'text/plain',
				[HEADER_TOKEN]:csrf
			}).then((res)=>{
				if(res.ok) text(query('.user-profile .name'),input.value?input.value:"No author name yet");
				else dialogOn(query('dialog.error'));
			}).catch(_=>{
				dialogOn(query('dialog.error'));
			}).finally(_=>{enable(input);unbusy(label);});
		});
	});
};

/** @type {function(function():void):void} */
const setupDeleteEverything=(cb)=>{
	const delete_everything=query('#delete-everything');
	const dialog=instanciate(delete_everything);
	const body=query('body');
	const input=find(dialog,'input.delete-everything');
	const button=find(dialog,'button.delete-everything');

	const close=()=>{
		dialogOff(dialog);
	};

	click(find(body,'button.delete'),()=>{
		unbusy(button);
		enable(button);
		dialogOn(dialog);
		input.value="";
		attr(input,'aria-invalid',null);
	});

	click(find(dialog,'button.cancel'),close);

	click(button,()=>{
		if(input.value!=="delete everything")
		{
			attr(input,'aria-invalid',"true");
			return;
		}
		attr(input,'aria-invalid',null);
		busy(button);
		disable(button);
		delay(200,_=>{
			post('delete_everything','',{
				[HEADER_TOKEN]:csrf
			}).then(ans=>{
				if(ans.ok) window.location.href="/community.html";
				else return Promise.reject();
			}).catch(_=>{
				showError();
			});
		});
	});

	append(body,dialog);
};

setupError();
// setupDeleteEverything();

setupSign().then(user=>{
	if(user) setupSetting(user);
	else window.location.href='/community.html';
});

});

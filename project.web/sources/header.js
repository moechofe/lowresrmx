const setupError=()=>{
	const error=query('#error');
	const dialog=instanciate(error);
	query('body').append(dialog);
}

const showError=()=>{
	dialogOn(query('diaflog.error'));
}

const setupSign=()=>{
	post('is_signed','')
	.then((res)=>res.json())
	.then((signed)=>{
		queryAll(signed===false?'.is-not-signed':'.is-signed').forEach((item)=>show(item));
		if(signed)
		{
			text(query('.user-profile .name'),signed.author?signed.author:"no author name yet");
			// query('.user-profile .picture').style.backgroundImage="url('"+signed.picture+"')";
		}
	}).catch((_)=>queryAll('.is-not-signed').forEach((item)=>show(item))).finally((_)=>queryAll('.is-loading').forEach((item)=>hide(item)));

	click(query('button.google-sign-in'),(_)=>{
		window.location.href='/google';
	});

	click(query('button.discord-sign-in'),(_)=>{
		window.location.href='/discord';
	});

	click(query('.sign-out a'),(_)=>{
		post('sign_out','')
		.then((res)=>res.json())
		.then((signed_out)=>{
			log("signed",!signed_out);
			window.location.reload();
		});
	});
};

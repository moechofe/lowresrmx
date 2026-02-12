((d)=>{
	d.addEventListener('DOMContentLoaded',()=>{
		console.log("sign-in.js loaded");
		const query=(q)=>d.querySelector(q)||create('div');
		const click=(e,c)=>{e.addEventListener('click',c)};
		click(query('button.google-sign-in'),()=>{
			console.log("google sign-in clicked");
			window.location.href='/google?uptoken='+query('section.card').dataset.uptoken;
		});
		click(query('button.discord-sign-in'),()=>{
			console.log("discord sign-in clicked");
			window.location.href='/discord?uptoken='+query('section.card').dataset.uptoken;
		});
	});
})(document);

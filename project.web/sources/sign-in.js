((d)=>{
	d.addEventListener('DOMContentLoaded',()=>{
		const query=(q)=>d.querySelector(q)||create('div');
		const click=(e,c)=>{e.addEventListener('click',c)};
		click(query('button.google-sign-in'),()=>{
			window.location.href='/google?uptoken='+query('section.card').dataset.uptoken;
		});
		click(query('button.discord-sign-in'),()=>{
			window.location.href='/discord?uptoken='+query('section.card').dataset.uptoken;
		});
	});
})(document);

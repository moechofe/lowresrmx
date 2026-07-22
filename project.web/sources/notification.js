// <?php
// 	require_once __DIR__.'/config.js';
// ?>

/**
 * @typedef {{
 * pid: string,
 * name: string?,
 * title: string?
 * author: string,
 * ut: string?
 * ct: string?,
 * eid: string?
 * pts: number?
 * }}
 */
var ProgramItem;

/**
 * @typedef {{
 * eid: string,
 * unseed: boolean,
 * title: string,
 * author: string,
 * comm: number,
 * vote: number,
 * }}
 */
var NotifItem;

/** @type NotifItem[] */
var notif;

/** @type function():Promise */
const retrieveNotifMaybe=()=>{return new Promise((res,rej)=>{
	console.log("retrieveNotifMaybe");
	const nt=sessionStorage.getItem("notif_time") || Date.now();
	const to_old=(Date.now()-nt)>60*60*15;
	if(!sessionStorage.getItem("notifs") || to_old)
	{
		get('/notif',{
			[HEADER_TOKEN]:csrf
		}).then((ans)=>{
			if(!ans.ok) return rej();
			return ans.json();
		}).then((list)=>{
			console.log("retrieveNotifMaybe",list);
			sessionStorage.setItem("notifs",JSON.stringify(list));
			sessionStorage.setItem("notif_time",Date.now())
			notif=list;
			res();
		});
	}
	else
	{
		try{notif=JSON.parse(sessionStorage.getItem("notifs"));}
		catch(e){notif=[];}
		res();
	}
})};

/** @type function(HTMLElement) */
const injectNotifMarker=(list)=>{
	console.log("inject",notif,list);
	for(let i=0;i<notif.length;++i)
	{
		if(notif[i].unseed)
		{
			const article=find(list,'article[data-eid="'+notif[i].eid+'"]');
			show(find(article,'.notif'));
		}
	}
};

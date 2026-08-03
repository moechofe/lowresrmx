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
 * unseen: boolean,
 * title: string,
 * author: string,
 * comm: number,
 * vote: number,
 * }}
 */
var NotifItem;

/** @type NotifItem[] */
let notif=[];

/** @type function():Promise */
const retrieveNotifMaybe=(is_signed)=>{return new Promise((res,rej)=>{
	if(!is_signed) return rej();
	const now=Math.floor(Date.now()/1000)
	const nt=sessionStorage.getItem("notif_time") || now;
	const to_old=(now-nt)>60*60*15;
	if(!sessionStorage.getItem("notifs") || to_old)
	{
		get('/notif',{
			[HEADER_TOKEN]:csrf
		}).then((ans)=>{
			if(!ans.ok) return rej();
			return ans.json();
		}).then((list)=>{
			sessionStorage.setItem("notifs",JSON.stringify(list));
			sessionStorage.setItem("notif_time",now)
			notif=list;
			res(notif);
		});
	}
	else
	{
		try{notif=JSON.parse(sessionStorage.getItem("notifs"));}
		catch(e){notif=[];}
		res(notif);
	}
})};

/** @type function(HTMLElement) */
const injectNotifMarker=(list)=>{
	for(let i=0;i<notif.length;++i)
	{
		if(notif[i].unseen)
		{
			const article=find(list,'article[data-eid="'+notif[i].eid+'"]');
			show(find(article,'.notif'));
		}
	}
};

const markNotifSeen=(eid)=>{
	if(sessionStorage.getItem("notifs"))
	{
		let notif=[];
		try{notif=JSON.parse(sessionStorage.getItem("notifs"));}
		catch(e){notif=[];}
		for(let i=0;i<notif.length;++i)
		{
			if(notif[i].eid==eid)
			{
				notif[i].unseen=false;
				sessionStorage.setItem("notifs",JSON.stringify(notif));
				return;
			}
		}
	}
};

<?php
	require_once __DIR__.'/common.php';
?>
(()=>new Promise(function(ready){
	document.addEventListener('readystatechange',ready)
}))().then(async()=>{

<?php
	require_once __DIR__.'/config.js';
	require_once __DIR__.'/dom.js';
	require_once __DIR__.'/common.js';
?>

/**
 * @typedef {{
 * text: string,
 * author: string,
 * ct: string,
 * }}
 */
var CommentItem;

const eid=dataget(query('article'),'eid');
let cid=0;

const setupDate=()=>{
	queryAll('.date[data-ct]').forEach((elem)=>{
		humanDate(elem,dataget(elem,'ct'));
	});
};

const setupCommentForm=()=>{
	const form=query('#comment');

	const showLimit=()=>{
		const limit=find(form,'.limit');
		const ta=find(form,'textarea');
		const max=dataget(limit,'limit');
		if(!max) return;
		text(limit,`${ta.value.length}/${max}`);
	};

	input(find(form,'textarea'),(event)=>{
		const max=dataget(find(form,'.limit'),'limit');
		const ta=find(form,'textarea');
		if(ta.value.length>max) ta.value=ta.value.substring(0,max);
		showLimit();
		autoHeight(event.target,214);
		event.target.scrollIntoView(true);
	});
	showLimit();
	autoHeight(find(form,'textarea'),214);

	click(find(form,'.comment'),async()=>{
		if(!eid) return;
		const text=find(form,'textarea').value.trim();
		if(!text) return;
		if(await post(`${eid}/comment`,text,{
			[HEADER_TOKEN]:csrf
		}))
		{
			find(form,'textarea').value='';
			window.location.reload();
		}
	});
};

/**
 * @brief Add comments to the list.
 * @param {!Array<!CommentItem>} cmnt_list List of comments.
 * @return {HTMLElement} The list of comments.
 * @event ask_for_more {number} The number of comments to skip, and load the following ones.
 */
const addComments=(cmnt_list)=>{
	const list=query('#comment-list');
	const item_tpl=query('#comment-item');

	/** @type {!Array<!HTMLElement>} */
	const items=cmnt_list.map(data=>{
		const item=instanciate(item_tpl);

		find(item,'.author > span').textContent=data.author;
		humanDate(find(item,'.date'),data.ct);
		find(item,'.text').innerHTML=data.text;
		return item;
	});

	list.append.apply(list,items);
	return list;
};

const setupVote=()=>{
	const vote=query('#vote');
	const points=query('.points');
	const upvoted=query('.upvoted');
	const upv_tpl=query('#upv');

	on(vote,'change',async()=>{
		if(!eid) return;
		disable(vote);
		const resp=await get(`/${eid}/vote`,{
			[HEADER_TOKEN]:csrf
		});
		if(resp.status===200)
		{
			const json=await resp.json();
			vote.checked=json.upv;
			text(points,json.pts);
			if(json.upv)
				for(i=0;i<20;++i)
					append(upvoted,instanciate(upv_tpl));
			else clear(upvoted);
		}
		else
		{
			delay(100,()=>vote.checked=!vote.checked);
		}
		enable(vote);
	});
};

const setupEntry=()=>{
	const open=query('#open');
	on(open,'click',()=>{
		const id=dataget(open,'id');
		const name=dataget(open,'name');
		if(id && name) window.open(`<?=APP_SCHEME?>\/\/?i=${encodeURIComponent(id)}&n=${encodeURIComponent(name)}`);
	});
	const player=query('#player');
	on(player,'click',()=>{
		const id=dataget(player,'id');
		if(id) window.open(`${encodeURIComponent(id)}.player`,'_blank');
	});
};

setupDate();
setupSign();
setupMobile();
setupVote();
setupError();
setupCommentForm();
setupEntry();

// TODO: show a loading stuff
const first_comment=await(await get(`/${eid}/${cid}`)).json();
addComments(first_comment);

// TODO: handle more comments

});

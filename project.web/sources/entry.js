(()=>new Promise(function(ready){
	document.addEventListener('readystatechange',ready)
}))().then(async()=>{

<?php require_once __DIR__.'/common.js'; ?>
<?php require_once __DIR__.'/header.js'; ?>

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
		limit.textContent=`${ta.value.length}/${max}`;
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
		if(await post(`${eid}/comment`,text,null))
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

		find(item,'.author').textContent=data.author;
		humanDate(find(item,'.date'),data.ct);
		find(item,'p').textContent=data.text;

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
		const resp=await get(`/${eid}/vote`);
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
		enable(vote);
	});
}

setupDate();
setupSign();
setupMobile();
setupVote();
setupError();
setupCommentForm();

// TODO: show a loading stuff
const first_comment=await(await get(`/${eid}/${cid}`)).json();
addComments(first_comment);

// TODO: handle more comments

});

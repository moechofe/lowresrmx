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
		renderReactions(find(item,'.reactions'),`${eid}/${data.cid}`,data.reactions);
		return item;
	});

	list.append.apply(list,items);
	return list;
};

const setupVote=(is_signed)=>{
	if(!is_signed) return;
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

	enable(vote);
};

/**
 * Emoji reactions. Rebuilds a reaction bar from a list of {e,n,mine}. Assigned
 * inside setupReactions (which captures is_signed); a no-op until then.
 * @type {function(!HTMLElement,string,!Array<!Object>):void}
 */
let renderReactions=(_container,_base,_list)=>{};

const setupReactions=(is_signed)=>{
	const chip_tpl=query('#reaction-chip');
	const dialog=query('#emoji-dialog');
	/** @type {?HTMLElement} */
	let picker=null;
	/** @type {?string} */
	let active_base=null;

	// A target's bar is the .reactions whose data-base matches (post: eid, comment: eid/cid).
	const findBase=(base)=>{
		let found=null;
		queryAll('.reactions').forEach((c)=>{ if(dataget(c,'base')===base) found=c; });
		return found;
	};

	renderReactions=(container,base,list)=>{
		clear(container);
		dataset(container,'base',base);
		(list||[]).forEach((r)=>{
			const chip=instanciate(chip_tpl);
			dataset(chip,'e',r.e);
			find(chip,'.e').textContent=r.e;
			find(chip,'.n').textContent=r.n;
			addClassCond(chip,'mine',!!r.mine);
			append(container,chip);
		});
		if(is_signed)
		{
			const add=create('button');
			add.className='react-add';
			attr(add,'aria-label','Add reaction');
			add.textContent='+';
			append(container,add);
		}
	};

	const toggleReact=async(base,emoji)=>{
		if(!base||!emoji) return;
		const resp=await post(`${base}/react`,emoji,{[HEADER_TOKEN]:csrf});
		if(resp.status===200)
		{
			const json=await resp.json();
			const container=findBase(base);
			if(container) renderReactions(container,base,json.reactions);
		}
	};

	const openPicker=(base)=>{
		active_base=base;
		if(!picker)
		{
			picker=create('emoji-picker');
			attr(picker,'data-source','/emoji-data.json');
			on(picker,'emoji-click',(event)=>{
				const unicode=event.detail&&event.detail.unicode;
				dialogOff(dialog);
				toggleReact(active_base,unicode);
			});
			append(find(dialog,'.picker-host'),picker);
		}
		dialogOn(dialog);
	};

	click(find(dialog,'.close'),()=>dialogOff(dialog));

	// Signed-out users see the bars read-only (no + button, no toggling).
	if(!is_signed) return;

	on(query('article'),'click',(event)=>{
		const chip=event.target.closest('.reaction');
		if(chip)
		{
			const container=chip.closest('.reactions');
			if(container) toggleReact(dataget(container,'base'),dataget(chip,'e'));
			return;
		}
		const add=event.target.closest('.react-add');
		if(add)
		{
			const container=add.closest('.reactions');
			if(container) openPicker(dataget(container,'base'));
		}
	});
};

const setupEntry=()=>{
	const open=query('#open');
	on(open,'click',()=>{
		const id=dataget(open,'id');
		const name=dataget(open,'name');
		if(id && name) window.location.href=`<?=APP_SCHEME?>\/\/?i=${encodeURIComponent(id)}&n=${encodeURIComponent(name)}`;
	});
	const player=query('#player');
	on(player,'click',()=>{
		const id=dataget(player,'id');
		if(id) window.open(`${encodeURIComponent(id)}.player`,'_blank');
	});
};

setupDate();
setupSign().then((is_signed)=>{
	setupVote(is_signed);
	setupReactions(is_signed);
});
setupMobile();
setupError();
setupCommentForm();
setupEntry();

// TODO: redo this
const first_comment=await(await get(`/${eid}/${cid}`)).json();
addComments(first_comment);

});

(()=>new Promise(function(ready){
	document.addEventListener('readystatechange',ready)
}))().then(async()=>{

<?php require_once __DIR__.'/common.js'; ?>
<?php require_once __DIR__.'/utils.js'; ?>

/**
 * @typedef {{
 * text: string,
 * author: string,
 * ct: string,
 * }}
 */
var CommentItem;

const eid=dataget(query('#comment-list'),'eid');
let cid=0;

const setupCommentForm=()=>{
	const form=query('#comment');

	click(find(form,'.comment'),()=>{
		const eid=dataget(form,'eid');
		if(!eid) return;
		const text=find(form,'textarea').value.trim();
		if(!text) return;
		post('/comment',JSON.stringify({
			f:eid,
			x:text,
		}),null);
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

		find(item,'.text').textContent=data.text;
		find(item,'.author').textContent=data.author;
		humanDate(find(item,'.date'),data.ct);

		return item;
	});

	list.append.apply(list,items);
	return list;
};

setupSign();
setupCommentForm();

// TODO: show a loading stuff
const first_comment=await(await get(`/${eid}/${cid}`)).json();
addComments(first_comment);

// TODO: handle more comments

});

/** @typedef {{
 * pid: string,
 * title: string,
 * author: string,
 * ct: string,
 * upvote: number,
 * }} */
var FirstItem;

const setupFirstList=(first_list)=>{return new Promise(async (res,rej)=>{
	const item_tpl=query("#first-item");
	const list=query("main.first-list");

	// List of first entry

	/**	@type {!Array<!HTMLElement>} */
	const items=first_list.map((data)=>{
		const item=instanciate(item_tpl);

		dataset(item,"pid",data.pid);

		text(find(item,".title"),data.title||"Untitled");
		find(item,".picture").style.backgroundImage="url(\"./"+data.pid+".png\")";
		humanDate(find(item,'.date'),data.ct);
		text(find(item,".author"),data.author);
		text(find(item,".points"),data.upvote);

		return item;
	});

	list.append.apply(list,items);

	res(list);
})};


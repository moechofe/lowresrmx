const setupProgramList=()=>{return new Promise(async function(res,rej){
	const program_list=query('#program-list');
	const program_item=query('#program-item');
	const list=instanciate(program_list);
	const body=query('body');

	/** @typedef {{
	 * pid: string,
	 * name: string,
	 * author: string,
	 * ct: string,
	 * }} */
	var ProgramItem;

	// List of programs

	/** @type {!Array<!ProgramItem>} */
	const my_last_prg=await(await get('my_last_prg')).json();

	/** @type {!Array<!HTMLElement>} */
	const items=my_last_prg.map((data)=>{
		const item=instanciate(program_item);
		const publish=find(item,'button.publish');

		find(item,'.name').textContent=data.name||"Untitled";

		// @type {!Date}
		var date=null;
		if(data.ct)
		{
			// Date in ISO 8601 format
			const epoch=Date.parse(data.ct);
			if(epoch) date=new Date(epoch);
		}
		if(date)
			// TODO: get the locale from the user throught cookie or sessions
			find(item,'.date').textContent=date.toLocaleDateString('en-US',{year:'numeric',month:'short',day:'numeric'});
		else
			find(item,'.date').textContent="Unknown date";

		// TODO: find a way to open the App with the program
		if(/Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent))
			find(item,'.play').remove();

		dataset(publish,'pid',data.pid);

		click(find(item,'.delete'),()=>{
			post('/delete',JSON.stringify({
				p:data.pid,
			}),null).then(()=>{
				item.remove();
			});
		});

		return item;
	});
	log("items",items);

	// List container
	list.append.apply(list,items);
	body.append(list);

	res(list);
});};

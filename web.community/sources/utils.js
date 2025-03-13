/** @type {function(!HTMLElement,string):void} */
const humanDate=(elem,timestamp)=>{
	/** @type {!Date} */
	var date=null;
	if(timestamp)
	{
		// Date in ISO 8601 format
		const epoch=Date.parse(timestamp);
		if(epoch) date=new Date(epoch);
	}
	// TODO: get the locale from the user throught cookie or sessions
	text(elem,date?date.toLocaleDateString('en-US',{
		year:'numeric',
		month:'short',
		day:'numeric'
	}):"Unknown date");
};

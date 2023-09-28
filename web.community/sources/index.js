const doc=document;
/** @type {function(string):!HTMLElement} */
const $=doc.querySelector.bind(doc);
const log=console.log.bind(console);

/** @type {function(string):!HTMLElement} */
const instanciate=(tpl)=>tpl.content.cloneNode(true).firstElementChild;

/** @type {function(!HTMLElement,function(!Event):void):void} */
const click=(element,callback)=>{element.addEventListener('click',callback)};

/** @type {function(!File):!Promise} */
const uploadFile=(file)=>{
	const reader=new FileReader();
	const xhr=new XMLHttpRequest();
	return new Promise((res,rej)=>{
		reader.onload=async(event)=>{
			log("reader.load",event);
			const response=await fetch('upload.php',{
				method:'POST',
				mode:'cors',
				cache:'no-cache',
				credentials:'same-origin',
				headers:{
					'Content-Type':'application/x-binary',
				},
				redirect:'follow',
				referrerPolicy:'no-referrer',
				body:event.target.result,
			});
			log("response",response);
		};
		reader.readAsBinaryString(file);
	});
};

document.addEventListener("DOMContentLoaded",(event)=>{
	const body=$('body');
	const share_program_btn=$('#share-program');
	const upload_card_tpl=$("#upload-card");
	const blocker_tpl=$('#blocker');

	const blocker=instanciate(blocker_tpl);
	const block=()=>{blocker.style.display='block'}
	const unUblock=()=>{blocker.style.display='none'}
	unUblock();
	body.appendChild(blocker);

	/** @type {function(!PointerEvent):void} */
	const popupUploadCard=(event)=>{
		block();
		const clone=instanciate(upload_card_tpl);
		const rect=share_program_btn.getBoundingClientRect();
		const style=clone.style;
		style.right=body.clientWidth-rect.right+'px';
		style.top=rect.top+rect.height+'px';
		body.appendChild(clone);
		const reset=$('#upload-card-reset');
		const submit=$('#upload-card-submit');
		click(reset,(event)=>{
			clone.remove();
			unUblock();
		});
		click(submit,(event)=>{
			//clone.remove();
			//unUblock();
			const program=$('#program-file').files[0];
			uploadFile(program);
		});
	}

	click(share_program_btn,popupUploadCard);
});


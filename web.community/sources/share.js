(()=>new Promise((ready)=>{
	document.addEventListener('DOMContentLoaded',ready)
}))().then(async()=>{

<?php require_once __DIR__.'/common.js'; ?>
<?php require_once __DIR__.'/utils.js'; ?>
<?php require_once __DIR__.'/last_shared.js'; ?>

const delete_dialog=setupProgramDelete();
const publish_dialog=setupPublishDialog();

/** @type {!Array<!SharedItem>} */
const last_shared=await(await get('last_shared')).json();

const list=await setupProgramList(last_shared);

on(list,'ask_to_delete',(event)=>{
	delete_dialog(event.detail.pid,event.detail.name,()=>{
		emit(list,'remove_item',{pid:event.detail.pid});
	});
});

on(list,'ask_to_publish',(event)=>{
	publish_dialog(event.detail.pid,event.detail.name,()=>{
		log("published");
	});
});

});

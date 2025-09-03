(()=>new Promise((ready)=>{
	document.addEventListener('DOMContentLoaded',ready)
}))().then(async()=>{

<?php require_once __DIR__.'/common.js'; ?>
<?php require_once __DIR__.'/header.js'; ?>
<?php require_once __DIR__.'/list.js'; ?>

const delete_dialog=setupProgramDelete();
const publish_dialog=setupPublishDialog();

const setupShareList=()=>{
	get('last_shared').then((ans)=>{
		if(!ans.ok) return Promise.reject("");
		return ans.json();
	}).then((list)=>{
		return setupProgramList(list,{isShare:true});
	}).then(/** @param {!Array<!ProgramItem>} list */(list)=>{
		on(list,'ask_to_delete',(event)=>{
			delete_dialog(event.detail.pid,event.detail.name,()=>{
				emit(list,'remove_item',{pid:event.detail.pid});
			});
		});

		on(list,'ask_to_publish',(event)=>{
			publish_dialog(event.detail.pid,event.detail.name,(pid)=>{
				log("published",pid);
				window.location.href=`./${encodeURIComponent(pid)}.html`;
				// TODO: relocate to the post
			});
		});
	})
	.catch((_)=>{});
};

setupSign();
setupError();
setupShareList();

});

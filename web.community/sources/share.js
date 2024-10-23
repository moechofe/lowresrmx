(()=>new Promise(function(ready){
	document.addEventListener('DOMContentLoaded',ready)
}))().then(()=>{

<?php require_once __DIR__.'/common.js'; ?>
<?php require_once __DIR__.'/program_list.js'; ?>
<?php require_once __DIR__.'/publish_dialog.js'; ?>

	Promise.resolve()
	.then(setupProgramList)
	.then(setupPublishDialog)
	;
});

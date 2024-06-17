(()=>new Promise(function(ready){
	document.addEventListener('readystatechange',ready)
}))().then(()=>{

<?php require_once __DIR__.'/common.js'; ?>

	setupSign();
	setupUploadCard();

});

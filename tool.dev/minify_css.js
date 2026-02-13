const fs=require('node:fs');
const {stdout}=require('node:process');
const ccss=require('clean-css');
const cleaner=new ccss({});
stdout.write(cleaner.minify(fs.readFileSync(0,'utf-8')).styles)

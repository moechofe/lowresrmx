import fs from 'fs'

import markdownit from 'markdown-it'
import markdownItAnchor from 'markdown-it-anchor'
const slugify=s=>encodeURIComponent(String(s).toLowerCase().replace(/[\W-]+/g, '-').replace(/-+$/, ''));
const mdit = markdownit({
	html:true,
	xhtmlOut:true,
	breaks:true, // doesn't work inside links
}).use(markdownItAnchor,{slugify})

import path from 'path'
import { fileURLToPath } from 'url'
const scriptDir = path.dirname(fileURLToPath(import.meta.url))
const rootDir = path.resolve(scriptDir, '..')

const match_h4=/<h4.*?<code>(.*)<\/code><\/h4>/gm
const replace_h4=/<h4 id=/
const add_keyword_to_h4=(text)=>{
	return text.replace(match_h4, (match, g1)=>{
		const kreg = /([A-Z][A-Z.]*[A-Z])/g;
		let m;
		let keyword=[];
		while ((m = kreg.exec(g1)) !== null) {
			if (m.index === kreg.lastIndex) kreg.lastIndex++;
			keyword.push(m[1]);
		}
		const treg = /(=?[A-Z][A-Z. ]*[A-Z])/g;
		let title=[];
		while ((m = treg.exec(g1)) !== null) {
			if (m.index === treg.lastIndex) treg.lastIndex++;
			title.push(m[1]);
		}
		return match.replace(replace_h4,`<h4 data-keyword="${keyword.join(',')}" data-title="${title.join('/')}" id=`);
	});
};

let md = fs.readFileSync(path.join(rootDir, 'asset.dev', 'manual.md'), 'utf8').split('\n').slice(1);

md = md.filter(line => !line.trim().startsWith('TODO:')).join('\n')

const tokens = mdit.parse(md, {});
const toc = [];
for (let i = 0; i < tokens.length; i++) {
	if (tokens[i].type === 'heading_open') {
		const level = parseInt(tokens[i].tag.slice(1));
		const id = tokens[i].attrGet('id');
		const text = tokens[i + 1].children
			.filter(t => t.type === 'text' || t.type === 'code_inline' || t.type === 'html_inline')
			.map(t => t.content)
			.join('');
		toc.push({ level, id, text });
	}
}
const tocHtml = toc.map(h => `<li class="h${h.level}"><a href="#${h.id}">${h.text}</a></li>`).join('\n');

const css = fs.readFileSync(path.join(rootDir, 'project.web', 'sources', 'documentation.css'), 'utf8')
const html = add_keyword_to_h4(mdit.render(md).replace(/&lt;br&gt;/g, '<br />').replace(/ style="text-align:right"/g, ' class="right"').replace(/<br>/g,'<br />'));

const community = `<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" />
<title>Documentation — Retro Game Creator</title>
<link rel="stylesheet" href="documentation.css">
<link rel="stylesheet" href="documentation-www.css">
<link rel="icon" href="favicon.ico" type="image/x-icon">
<script src="documentation.js"></script>
</head>
<body>
<aside><nav><ul>
${tocHtml}
</ul></nav></aside>
<article>
${html}
</articl>
</body>
</html>`

const bundle = `
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
<title>Retro Game Creator - Documentation</title>
<style>
<!--
${css}
-->
</style>
<body>
${html}
</body>
</html>
`

let file=path.join(rootDir, 'project.web', 'sources', 'documentation.html');
fs.writeFileSync(file, community, 'utf8')
console.log(file);

file=path.join(rootDir, 'asset.manual', 'manual.html')
fs.writeFileSync(file, bundle, 'utf8')
console.log(file)

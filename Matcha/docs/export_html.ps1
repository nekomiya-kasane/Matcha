#!/usr/bin/env pwsh
# export_html.ps1 — Convert Matcha_Design_System_Specification.md to self-contained HTML
# Strategy: Read MD, base64-encode it, embed in HTML that decodes + renders client-side

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$mdPath    = Join-Path $scriptDir "Matcha_Design_System_Specification.md"
$outPath   = Join-Path $scriptDir "Matcha_Design_System_Specification.html"

# Read and base64-encode the markdown
$mdBytes  = [System.IO.File]::ReadAllBytes($mdPath)
$mdBase64 = [Convert]::ToBase64String($mdBytes)

# Build HTML in parts to avoid here-string interpolation issues
$htmlPart1 = @'
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Matcha Design System Specification</title>
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/github.min.css">
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/KaTeX/0.16.9/katex.min.css">
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/cpp.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/cmake.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/python.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/marked/12.0.2/marked.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/mermaid/10.9.1/mermaid.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/KaTeX/0.16.9/katex.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/KaTeX/0.16.9/contrib/auto-render.min.js"></script>
<style>
/* === Pastel Cotton Candy palette ===
 * Powder Blush  #ffadad   Apricot Cream #ffd6a5
 * Lemon Chiffon #fdffb6   Tea Green     #caffbf
 * Soft Cyan     #9bf6ff   Baby Blue Ice #a0c4ff
 * Periwinkle    #bdb2ff   Mauve         #ffc6ff
 * Porcelain     #fffffc
 */
:root {
  --bg: #fffffc;
  --fg: #2d2640;
  --heading: #3b2d5e;
  --link: #5b6abf;
  --link-hover: #8b5fbf;
  --border: #d8cfe8;
  --code-bg: #eaf8ff;
  --code-border: #b8e4f9;
  --table-bg: #fffcf5;
  --table-alt: #fff0d6;
  --table-header: #ffe5c2;
  --bq-bg: #eaffd8;
  --bq-border: #7dd87d;
  --imp-bg: #fff9d6;
  --imp-border: #e8a840;
  --sidebar-bg: linear-gradient(180deg, #f0ebff 0%, #ffe8f6 50%, #eaf8ff 100%);
  --sidebar-border: #dcc8f0;
  --accent-blush: #ffadad;
  --accent-cyan: #9bf6ff;
  --accent-periwinkle: #bdb2ff;
  --accent-mauve: #ffc6ff;
  --accent-green: #caffbf;
  --accent-lemon: #fdffb6;
}
* { box-sizing: border-box; margin: 0; padding: 0; }
body {
  font-family: "Segoe UI", -apple-system, BlinkMacSystemFont, "Helvetica Neue", Arial, sans-serif;
  font-size: 15px; line-height: 1.7; color: var(--fg); background: var(--bg); display: flex;
}
#sidebar {
  position: fixed; top: 0; left: 0; width: 280px; height: 100vh;
  overflow-y: auto; background: var(--sidebar-bg);
  border-right: 1px solid var(--sidebar-border);
  padding: 16px 12px; font-size: 13px; z-index: 100;
}
#sidebar h3 {
  font-size: 14px; margin-bottom: 10px; color: var(--heading);
  padding-bottom: 6px; border-bottom: 2px solid var(--accent-periwinkle);
}
#sidebar ul { list-style: none; padding-left: 0; }
#sidebar li { margin: 2px 0; }
#sidebar li.toc-h2 { padding-left: 0; font-weight: 600; margin-top: 6px; }
#sidebar li.toc-h3 { padding-left: 14px; }
#sidebar a {
  color: var(--fg); text-decoration: none; display: block;
  padding: 3px 6px; border-radius: 4px; transition: background 0.2s, color 0.2s;
}
#sidebar a:hover { background: var(--accent-mauve); color: var(--heading); }
#content {
  margin-left: 280px; max-width: 900px; padding: 40px 48px 80px;
}
h1, h2, h3, h4, h5, h6 {
  color: var(--heading); margin-top: 1.6em; margin-bottom: 0.6em; line-height: 1.3;
}
h1 {
  font-size: 2em; padding-bottom: 10px;
  border-bottom: 3px solid transparent;
  border-image: linear-gradient(90deg, var(--accent-blush), var(--accent-periwinkle), var(--accent-cyan)) 1;
}
h2 {
  font-size: 1.6em; padding-bottom: 6px;
  border-bottom: 2px solid transparent;
  border-image: linear-gradient(90deg, var(--accent-periwinkle), var(--accent-cyan), var(--accent-green)) 1;
}
h3 { font-size: 1.3em; }
h4 { font-size: 1.1em; }
p { margin: 0.6em 0; }
a { color: var(--link); text-decoration: none; transition: color 0.15s; }
a:hover { color: var(--link-hover); text-decoration: underline; }
code {
  font-family: "Cascadia Code", "JetBrains Mono", "Fira Code", Consolas, monospace;
  font-size: 0.9em; background: var(--code-bg);
  padding: 1px 5px; border-radius: 3px; border: 1px solid var(--code-border);
}
pre {
  background: var(--code-bg); border: 1px solid var(--code-border);
  border-radius: 8px; padding: 14px 18px; overflow-x: auto;
  margin: 1em 0; line-height: 1.5;
  border-left: 4px solid var(--accent-cyan);
}
pre code { background: transparent; border: none; padding: 0; font-size: 13px; }
table { border-collapse: collapse; width: 100%; margin: 1em 0; font-size: 14px; border-radius: 6px; overflow: hidden; }
th, td { border: 1px solid var(--border); padding: 8px 12px; text-align: left; }
th { background: var(--table-header); font-weight: 600; color: var(--heading); }
tr:nth-child(even) { background: var(--table-alt); }
tr:nth-child(odd) { background: var(--table-bg); }
blockquote {
  border-left: 4px solid var(--bq-border); background: var(--bq-bg);
  padding: 10px 16px; margin: 1em 0; border-radius: 6px;
}
blockquote p { margin: 4px 0; }
.important, .admonition-important {
  border-left: 4px solid var(--imp-border); background: var(--imp-bg);
  padding: 12px 16px; margin: 1em 0; border-radius: 6px;
}
ul, ol { margin: 0.5em 0; padding-left: 1.6em; }
li { margin: 0.2em 0; }
.mermaid {
  text-align: center; margin: 1.2em 0;
  background: #fffffc; padding: 12px; border-radius: 8px;
  border: 1px solid var(--border);
}
.katex-display { margin: 1em 0; overflow-x: auto; overflow-y: hidden; }
.katex { font-size: 1.1em; }
hr {
  border: none; margin: 2em 0; height: 2px;
  background: linear-gradient(90deg, var(--accent-blush), var(--accent-lemon), var(--accent-green), var(--accent-cyan), var(--accent-periwinkle), var(--accent-mauve));
}
img { max-width: 100%; height: auto; }
/* === Print: color-friendly, no sidebar === */
@media print {
  #sidebar { display: none; }
  #content { margin-left: 0; max-width: 100%; padding: 20px; }
  body { background: #fff; -webkit-print-color-adjust: exact; print-color-adjust: exact; color-adjust: exact; }
  pre { border-left-color: #9bf6ff !important; background: #f0faff !important; }
  th { background: #ffe5c2 !important; }
  tr:nth-child(even) { background: #fff5e6 !important; }
  blockquote { background: #eaffd8 !important; border-left-color: #7dd87d !important; }
  .admonition-important { background: #fff9d6 !important; border-left-color: #e8a840 !important; }
  h1, h2 { border-image: none !important; border-bottom: 2px solid #bdb2ff !important; }
  hr { background: linear-gradient(90deg, #ffadad, #fdffb6, #caffbf, #9bf6ff, #bdb2ff, #ffc6ff) !important; }
  a { color: #5b6abf !important; }
  .mermaid { border: 1px solid #d8cfe8 !important; break-inside: avoid; }
  .katex-display { break-inside: avoid; }
  table { break-inside: avoid; }
}
@media (max-width: 900px) { #sidebar { display: none; } #content { margin-left: 0; padding: 20px; } }
</style>
</head>
<body>
<nav id="sidebar"><h3>Table of Contents</h3><ul id="toc-list"></ul></nav>
<main id="content"><p>Loading...</p></main>
<script id="md-source" type="text/plain">
'@

$htmlPart2 = @'
</script>
<script>
mermaid.initialize({ startOnLoad: false, theme: 'default', securityLevel: 'loose' });

marked.setOptions({ gfm: true, breaks: false });

const renderer = new marked.Renderer();
renderer.code = function(code, language) {
  let text = typeof code === 'object' ? code.text : code;
  let lang = typeof code === 'object' ? code.lang : language;
  if (lang === 'mermaid') {
    return '<div class="mermaid">' + text + '</div>';
  }
  let highlighted;
  try {
    highlighted = (lang && hljs.getLanguage(lang))
      ? hljs.highlight(text, { language: lang }).value
      : hljs.highlightAuto(text).value;
  } catch(e) {
    highlighted = text.replace(/</g,'&lt;').replace(/>/g,'&gt;');
  }
  return '<pre><code class="hljs language-' + (lang||'') + '">' + highlighted + '</code></pre>';
};

// Decode base64 markdown
const b64 = document.getElementById('md-source').textContent.trim();
const bytes = Uint8Array.from(atob(b64), c => c.charCodeAt(0));
const mdSource = new TextDecoder('utf-8').decode(bytes);

// Preprocess ::: important admonitions
let processed = mdSource.replace(/::: important\n([\s\S]*?):::/g,
  '<div class="admonition-important">\n\n$1\n</div>\n');

// --- LaTeX protection: shield math from marked's inline formatting ---
// Marked treats _ as emphasis and { as HTML. We extract all LaTeX blocks
// into placeholders before parsing, then restore them after.
const mathStore = [];
function mathPlaceholder(id) { return '%%MATH_' + id + '%%'; }

// 1) Protect fenced code blocks first (so we don't touch ``` content)
const codeBlocks = [];
processed = processed.replace(/(```[\s\S]*?```)/g, function(m) {
  codeBlocks.push(m);
  return '%%CODE_' + (codeBlocks.length - 1) + '%%';
});

// 2) Protect display math $$...$$ (may span lines)
processed = processed.replace(/\$\$([\s\S]*?)\$\$/g, function(m, inner) {
  mathStore.push(m);
  return mathPlaceholder(mathStore.length - 1);
});

// 3) Protect inline math $...$ (single line, non-greedy)
processed = processed.replace(/\$([^$\n]+?)\$/g, function(m, inner) {
  mathStore.push(m);
  return mathPlaceholder(mathStore.length - 1);
});

// 4) Restore code blocks
processed = processed.replace(/%%CODE_(\d+)%%/g, function(m, id) {
  return codeBlocks[parseInt(id)];
});

// Parse with marked
let html = marked.parse(processed, { renderer: renderer });

// Restore math blocks
html = html.replace(/%%MATH_(\d+)%%/g, function(m, id) {
  return mathStore[parseInt(id)];
});

document.getElementById('content').innerHTML = html;

// Render mermaid
mermaid.run({ nodes: document.querySelectorAll('.mermaid') });

// Render KaTeX
renderMathInElement(document.getElementById('content'), {
  delimiters: [
    { left: '$$', right: '$$', display: true },
    { left: '$', right: '$', display: false }
  ],
  throwOnError: false,
  trust: true
});

// Build sidebar TOC
const tocList = document.getElementById('toc-list');
document.querySelectorAll('#content h2, #content h3').forEach(function(h) {
  if (!h.id) {
    h.id = h.textContent.trim().toLowerCase()
      .replace(/[^\w\s-]/g, '').replace(/\s+/g, '-');
  }
  const li = document.createElement('li');
  li.className = 'toc-' + h.tagName.toLowerCase();
  const a = document.createElement('a');
  a.href = '#' + h.id;
  a.textContent = h.textContent;
  li.appendChild(a);
  tocList.appendChild(li);
});
</script>
</body>
</html>
'@

# Combine: HTML part 1 + base64 markdown + HTML part 2
$sb = [System.Text.StringBuilder]::new($htmlPart1.Length + $mdBase64.Length + $htmlPart2.Length + 10)
[void]$sb.Append($htmlPart1)
[void]$sb.Append("`n")
[void]$sb.Append($mdBase64)
[void]$sb.Append("`n")
[void]$sb.Append($htmlPart2)

[System.IO.File]::WriteAllText($outPath, $sb.ToString(), [System.Text.UTF8Encoding]::new($false))
Write-Host "Exported: $outPath  ($(([System.IO.FileInfo]$outPath).Length / 1024) KB)"

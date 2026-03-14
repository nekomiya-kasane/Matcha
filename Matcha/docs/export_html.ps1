#!/usr/bin/env pwsh
# export_html.ps1 — Convert Matcha_Design_System_Specification.md to self-contained HTML
# Strategy: Read MD, base64-encode it, embed in HTML that decodes + renders client-side

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$mdPath    = Join-Path $scriptDir "Matcha_Design_System_Specification.md"
$outPath   = Join-Path $scriptDir "Matcha_Design_System_Specification.html"

# ---------------------------------------------------------------------------
# Inline playground iframes: replace <iframe src="playground/..." data-playground="slug">
# with <iframe srcdoc="..."> containing self-contained HTML (tokens + base + component)
# ---------------------------------------------------------------------------
$mdText = [System.IO.File]::ReadAllText($mdPath, [System.Text.Encoding]::UTF8)

$playgroundDir = Join-Path $scriptDir "playground"
$tokensCssPath = Join-Path $playgroundDir "tokens.css"
$baseCssPath   = Join-Path $playgroundDir "base.css"

$tokensCss = if (Test-Path $tokensCssPath) { [System.IO.File]::ReadAllText($tokensCssPath, [System.Text.Encoding]::UTF8) } else { "" }
$baseCss   = if (Test-Path $baseCssPath)   { [System.IO.File]::ReadAllText($baseCssPath, [System.Text.Encoding]::UTF8) } else { "" }

# Collect LiveCodes playground data: slug -> {html, css}
$pattern = '<div\s+class="livecodes"\s+data-playground="([^"]*)"[^>]*>'
$matches_ = [regex]::Matches($mdText, $pattern)
Write-Host "Found $($matches_.Count) LiveCodes playground(s)."

$playgroundData = @{}  # slug -> @{html=...; css=...}

foreach ($m in $matches_) {
    $slug = $m.Groups[1].Value
    $componentFile = "$slug.html"
    $componentPath = Join-Path $playgroundDir "components" $componentFile

    if (Test-Path $componentPath) {
        $componentHtml = [System.IO.File]::ReadAllText($componentPath, [System.Text.Encoding]::UTF8)
    } else {
        $componentHtml = "<p>Component not found: $componentFile</p>"
    }

    # Extract <style> from component file
    $compStyle = ""
    $compBody  = $componentHtml
    if ($componentHtml -match '(?s)<style[^>]*>(.*?)</style>') {
        $compStyle = $Matches[1]
        $compBody  = $componentHtml -replace '(?s)<style[^>]*>.*?</style>', ''
    }

    # CSS = tokens + base + component style
    $fullCss = $tokensCss + "`n`n" + $baseCss + "`n`n" + $compStyle
    # HTML = component body wrapped with data-theme root
    $fullHtml = '<div data-theme="light" data-density="default">' + "`n" + $compBody.Trim() + "`n</div>"

    $playgroundData[$slug] = @{
        html = $fullHtml
        css  = $fullCss
    }
    Write-Host "  Prepared: $slug — HTML $($fullHtml.Length) chars, CSS $($fullCss.Length) chars"
}

Write-Host "LiveCodes playgrounds prepared."

# Base64-encode the processed markdown
$mdBytes  = [System.Text.Encoding]::UTF8.GetBytes($mdText)
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
#sidebar ul { list-style: none; padding-left: 0; margin: 0; }
#sidebar li { margin: 1px 0; }
#sidebar li.toc-h1 { padding-left: 0; font-weight: 700; margin-top: 10px; font-size: 1.0em; color: var(--heading); }
#sidebar li.toc-h2 { padding-left: 0; font-weight: 600; margin-top: 4px; }
#sidebar li.toc-h3 { padding-left: 14px; }
#sidebar li.toc-h4 { padding-left: 28px; font-size: 0.92em; color: #666; }
#sidebar .toc-toggle {
  cursor: pointer; user-select: none; display: flex; align-items: center; gap: 4px;
  padding: 3px 6px; border-radius: 4px; transition: background 0.15s;
}
#sidebar .toc-toggle:hover { background: var(--accent-mauve); }
#sidebar .toc-toggle::before {
  content: '\25B6'; font-size: 8px; color: #999; transition: transform 0.2s; display: inline-block; width: 12px; flex-shrink: 0;
}
#sidebar .toc-toggle.expanded::before { transform: rotate(90deg); }
#sidebar .toc-children { overflow: hidden; max-height: 0; transition: max-height 0.3s ease; }
#sidebar .toc-children.open { max-height: 9999px; }
#sidebar .toc-leaf a, #sidebar .toc-children a {
  color: var(--fg); text-decoration: none; display: block;
  padding: 2px 6px 2px 18px; border-radius: 4px; transition: background 0.15s, color 0.15s;
}
#sidebar .toc-leaf a:hover, #sidebar .toc-children a:hover { background: rgba(139,95,191,0.15); color: var(--heading); }
#sidebar a {
  color: var(--fg); text-decoration: none; display: block;
  padding: 3px 6px; border-radius: 4px; transition: background 0.2s, color 0.2s;
}
#sidebar a:hover { background: var(--accent-mauve); color: var(--heading); }
#content {
  margin-left: 280px; max-width: 900px; padding: 40px 48px 80px;
}
a.spec-ref {
  color: var(--accent-periwinkle, #5b6abf); text-decoration: none;
  border-bottom: 1px dotted var(--accent-periwinkle, #5b6abf);
  transition: color 0.15s, border-color 0.15s;
}
a.spec-ref:hover {
  color: var(--accent-mauve, #8b5fbf);
  border-bottom-style: solid;
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
/* === Inline TOC in content === */
#inline-toc { margin: 1.2em 0 2em; padding: 24px 32px; background: linear-gradient(135deg, #f8f4ff 0%, #eaf8ff 50%, #f0fff0 100%); border: 1px solid var(--border); border-radius: 10px; }
#inline-toc > .toc-title { font-size: 1.1em; font-weight: 700; color: var(--heading); margin-bottom: 12px; padding-bottom: 8px; border-bottom: 2px solid var(--accent-periwinkle); }
#inline-toc ul { list-style: none; padding-left: 0; margin: 0; }
#inline-toc li { margin: 1px 0; line-height: 1.5; }
#inline-toc li.itoc-h1 { font-weight: 700; font-size: 1.0em; margin-top: 14px; padding-left: 0; color: var(--heading); }
#inline-toc li.itoc-h1:first-child { margin-top: 0; }
#inline-toc li.itoc-h2 { font-weight: 600; font-size: 0.95em; padding-left: 16px; margin-top: 4px; }
#inline-toc li.itoc-h3 { font-weight: 400; font-size: 0.88em; padding-left: 32px; color: #555; }
#inline-toc li.itoc-h4 { font-weight: 400; font-size: 0.84em; padding-left: 48px; color: #777; }
#inline-toc a { color: var(--fg); text-decoration: none; transition: color 0.15s; }
#inline-toc a:hover { color: var(--link-hover); text-decoration: underline; }
iframe[data-playground] {
  width: 100%; border: 1px solid var(--border); border-radius: 8px;
  background: #fff; margin: 1em 0;
}
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

// ---- Assign IDs to all headings ----
document.querySelectorAll('#content h1, #content h2, #content h3, #content h4, #content h5').forEach(function(h) {
  if (!h.id) {
    h.id = h.textContent.trim().toLowerCase()
      .replace(/[^\w\s-]/g, '').replace(/\s+/g, '-');
  }
});

// ---- Auto-link §x.x / Ch.x / Chapter x references to heading anchors ----
(function linkSpecReferences() {
  // Build heading-number -> id lookup from all headings
  var headingMap = {};  // e.g. "2b.1" -> "2b1-spacing-tokens", "8.7" -> "87-interaction-timing-tokens-"
  document.querySelectorAll('#content h1, #content h2, #content h3, #content h4, #content h5').forEach(function(h) {
    var txt = h.textContent.trim();
    // Match: "AI.1 ...", "2b.1 ...", "Chapter 10. ...", "3.4 ..." etc.
    var m = txt.match(/^(?:Chapter\s+)?(\d+[a-z]?(?:\.\d+)*)/i) || txt.match(/^(AI(?:\.\d+)*)/);
    if (m) headingMap[m[1]] = h.id;
  });

  // Pattern groups: (1)§AI.x (2)AI range-end | (3)§num.num (4)num range-end | (5)Ch.num | (6)Chapter num
  var refRe = /(?:§(AI(?:\.\d+)*)(?:\s*[–\-]\s*(AI(?:\.\d+)*))?|§(\d+[a-z]?(?:\.\d+)*)(?:\s*[–\-]\s*(\d+[a-z]?(?:\.\d+)*))?|Ch\.(\d+)|Chapter\s+(\d+))/g;

  function sectionToId(sec) {
    if (headingMap[sec]) return headingMap[sec];
    // Try progressively shorter prefixes: "7.14.3" -> "7.14" -> "7"
    var parts = sec.split('.');
    while (parts.length > 1) {
      parts.pop();
      var prefix = parts.join('.');
      if (headingMap[prefix]) return headingMap[prefix];
    }
    return null;
  }

  function chapterToId(num) {
    // Look for "chapter-NUM-..." pattern in heading IDs
    var prefix = 'chapter-' + num + '-';
    var ids = Object.values(headingMap);
    for (var i = 0; i < ids.length; i++) {
      if (ids[i].indexOf(prefix) === 0) return ids[i];
    }
    // Also check direct number match (Part I chapters: "1", "2", etc.)
    return headingMap[num] || null;
  }

  // Process text-bearing elements inside #content
  document.querySelectorAll('#content td, #content p, #content li, #content blockquote').forEach(function(el) {
    if (el.closest('h1, h2, h3, h4, h5, a')) return;
    var html = el.innerHTML;
    var changed = false;
    var newHtml = html.replace(refRe, function(match, ai, aiEnd, sec, secEnd, chDot, chWord) {
      var targetId = null;
      if (ai) {
        targetId = sectionToId(ai);
      } else if (sec) {
        targetId = sectionToId(sec);
      } else if (chDot) {
        targetId = chapterToId(chDot);
      } else if (chWord) {
        targetId = chapterToId(chWord);
      }
      if (targetId) {
        changed = true;
        return '<a href="#' + targetId + '" class="spec-ref">' + match + '</a>';
      }
      return match;
    });
    if (changed) el.innerHTML = newHtml;
  });
})();

// ---- Widget spec 12-section headings to exclude from TOC ----
var widgetSpecHeadings = new Set([
  'Synopsis', 'Anatomy', 'Usage Guidelines',
  'Theme-Customizable Properties', 'Variant x State Token Mapping',
  'Notification Catalog', 'UiNode Public API', 'Animation',
  'Mathematical Model', 'Accessibility'
]);
function isWidgetSpecHeading(text) {
  var t = text.replace(/\s*\u{1F195}$/u, '').trim();  // strip trailing 🆕
  return widgetSpecHeadings.has(t);
}

// ---- Build collapsible sidebar TOC ----
(function buildSidebarTOC() {
  var tocList = document.getElementById('toc-list');
  var headings = Array.from(document.querySelectorAll('#content h1, #content h2, #content h3'));
  // Filter out widget spec sub-headings and TOC heading itself
  headings = headings.filter(function(h) {
    if (h.textContent.trim() === 'Table of Contents') return false;
    if (isWidgetSpecHeading(h.textContent)) return false;
    return true;
  });

  var i = 0;
  while (i < headings.length) {
    var h = headings[i];
    var tag = h.tagName;  // H1, H2, H3
    var li = document.createElement('li');
    li.className = 'toc-' + tag.toLowerCase();

    // Collect children: for H1, children are following H2s (and their H3s).
    // For H2, children are following H3s. H3 has no children in sidebar.
    var childLevel = (tag === 'H1') ? 'H2' : (tag === 'H2') ? 'H3' : null;
    var children = [];
    if (childLevel) {
      var j = i + 1;
      while (j < headings.length) {
        var ct = headings[j].tagName;
        // Stop at same or higher level
        if (ct === tag || (tag === 'H2' && ct === 'H1') || (tag === 'H1' && false)) break;
        if (ct === childLevel) children.push(headings[j]);
        // For H1 parent, also collect H3 as grandchildren (they'll be nested under H2)
        if (tag === 'H1' && ct === 'H3') { /* skip, will be handled by H2 parent */ }
        j++;
      }
    }

    if (children.length > 0 && (tag === 'H1' || tag === 'H2')) {
      // Collapsible node
      var toggle = document.createElement('div');
      toggle.className = 'toc-toggle';
      var a = document.createElement('a');
      a.href = '#' + h.id;
      a.textContent = h.textContent;
      a.addEventListener('click', function(e) { e.stopPropagation(); });
      toggle.appendChild(a);
      li.appendChild(toggle);

      var childUl = document.createElement('ul');
      childUl.className = 'toc-children';
      // Default: H1 nodes expanded, H2 nodes collapsed
      if (tag === 'H1') {
        toggle.classList.add('expanded');
        childUl.classList.add('open');
      }
      toggle.addEventListener('click', function(ul, tgl) {
        return function() { ul.classList.toggle('open'); tgl.classList.toggle('expanded'); };
      }(childUl, toggle));

      // We only add direct children here; H3 under H2 is handled when H2 is processed
      // So for H2 parent, just add H3 children as flat list
      if (tag === 'H2') {
        children.forEach(function(ch) {
          var cli = document.createElement('li');
          cli.className = 'toc-h3';
          var ca = document.createElement('a');
          ca.href = '#' + ch.id;
          ca.textContent = ch.textContent;
          cli.appendChild(ca);
          childUl.appendChild(cli);
        });
      }
      li.appendChild(childUl);
    } else {
      // Leaf node
      li.classList.add('toc-leaf');
      var a2 = document.createElement('a');
      a2.href = '#' + h.id;
      a2.textContent = h.textContent;
      li.appendChild(a2);
    }

    // Append to correct parent
    if (tag === 'H3') {
      // H3 is handled as child of H2, skip standalone append
      i++; continue;
    }
    if (tag === 'H2') {
      // Find the last H1 li's childUl to append to, or append to root
      var lastH1Ul = tocList.querySelector(':scope > li.toc-h1:last-child > .toc-children');
      if (lastH1Ul) {
        lastH1Ul.appendChild(li);
      } else {
        tocList.appendChild(li);
      }
    } else {
      tocList.appendChild(li);
    }
    i++;
  }
})();

// ---- Build inline TOC (replaces "Table of Contents" placeholder) ----
(function buildInlineTOC() {
  var tocHeading = null;
  document.querySelectorAll('#content h2').forEach(function(h) {
    if (h.textContent.trim() === 'Table of Contents') tocHeading = h;
  });
  if (!tocHeading) return;

  var headings = Array.from(document.querySelectorAll('#content h1, #content h2, #content h3, #content h4'));
  var tocIdx = headings.indexOf(tocHeading);
  var entries = headings.slice(tocIdx + 1).filter(function(h) {
    return !isWidgetSpecHeading(h.textContent);
  });

  var container = document.createElement('nav');
  container.id = 'inline-toc';
  var title = document.createElement('div');
  title.className = 'toc-title';
  title.textContent = 'Table of Contents';
  container.appendChild(title);

  var ul = document.createElement('ul');
  entries.forEach(function(h) {
    var li = document.createElement('li');
    li.className = 'itoc-' + h.tagName.toLowerCase();
    var a = document.createElement('a');
    a.href = '#' + h.id;
    a.textContent = h.textContent;
    li.appendChild(a);
    ul.appendChild(li);
  });
  container.appendChild(ul);

  var sibling = tocHeading.nextElementSibling;
  var toRemove = [];
  while (sibling && sibling.tagName !== 'H1' && sibling.tagName !== 'H2' && sibling.tagName !== 'HR') {
    toRemove.push(sibling);
    sibling = sibling.nextElementSibling;
  }
  toRemove.forEach(function(el) { el.remove(); });
  tocHeading.replaceWith(container);
})();

// Initialize LiveCodes playgrounds
(function() {
  var pgData = window.__playgroundData || {};
  var containers = document.querySelectorAll('.livecodes[data-playground]');
  if (containers.length === 0) return;

  // Load LiveCodes SDK from CDN
  var script = document.createElement('script');
  script.type = 'module';
  var initCode = '';
  containers.forEach(function(el) {
    var slug = el.getAttribute('data-playground');
    var data = pgData[slug];
    if (!data) { el.innerHTML = '<p style="color:#999">Playground data not found: ' + slug + '</p>'; return; }
    el.id = 'livecodes-' + slug;
  });

  // Build module script that imports LiveCodes and creates playgrounds
  var moduleScript = document.createElement('script');
  moduleScript.type = 'module';
  moduleScript.textContent = [
    'import { createPlayground } from "https://cdn.jsdelivr.net/npm/livecodes@0.7.0/livecodes.js";',
    'var pgData = window.__playgroundData;',
    'document.querySelectorAll(".livecodes[data-playground]").forEach(async function(el) {',
    '  var slug = el.getAttribute("data-playground");',
    '  var data = pgData[slug];',
    '  if (!data) return;',
    '  try {',
    '    await createPlayground(el, {',
    '      appUrl: "https://v35.livecodes.io/",',
    '      loading: "lazy",',
    '      config: {',
    '        markup:  { language: "html", content: data.html },',
    '        style:   { language: "css",  content: data.css },',
    '        activeEditor: "markup",',
    '      }',
    '    });',
    '  } catch(e) { console.warn("LiveCodes init failed for " + slug, e); }',
    '});'
  ].join('\n');
  document.body.appendChild(moduleScript);
})();
</script>
</body>
</html>
'@

# Build playground data as JSON for injection into the HTML
# Each entry: { slug: { html: "...", css: "..." } }
# We JSON-encode with care to avoid breaking the JS string literals
$pgEntries = @()
foreach ($key in $playgroundData.Keys) {
    $htmlEsc = $playgroundData[$key].html -replace '\\', '\\\\' -replace '"', '\"' -replace "`n", '\n' -replace "`r", ''  -replace '</script>', '<\/script>'
    $cssEsc  = $playgroundData[$key].css  -replace '\\', '\\\\' -replace '"', '\"' -replace "`n", '\n' -replace "`r", ''  -replace '</script>', '<\/script>'
    $pgEntries += "`"$key`":{`"html`":`"$htmlEsc`",`"css`":`"$cssEsc`"}"
}
$pgJson = "{" + ($pgEntries -join ",") + "}"

# Inject playground data at the top of the render script
$pgInject = "window.__playgroundData=$pgJson;`n"
$htmlPart2 = $htmlPart2.Replace(
    "mermaid.initialize(",
    $pgInject + "mermaid.initialize("
)

# Combine: HTML part 1 + base64 markdown + HTML part 2
$sb = [System.Text.StringBuilder]::new($htmlPart1.Length + $mdBase64.Length + $htmlPart2.Length + 10)
[void]$sb.Append($htmlPart1)
[void]$sb.Append("`n")
[void]$sb.Append($mdBase64)
[void]$sb.Append("`n")
[void]$sb.Append($htmlPart2)

[System.IO.File]::WriteAllText($outPath, $sb.ToString(), [System.Text.UTF8Encoding]::new($false))
Write-Host "Exported: $outPath  ($(([System.IO.FileInfo]$outPath).Length / 1024) KB)"

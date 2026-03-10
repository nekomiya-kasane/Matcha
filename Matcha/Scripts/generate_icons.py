#!/usr/bin/env python3
"""Generate all framework SVG icons for Matcha icon system (B2)."""

import os

ICONS_DIR = os.path.join(os.path.dirname(__file__), "..", "Resources", "Icons")

H = '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">'
T = "</svg>\n"


def s(body):
    return H + body + T


ICONS = {
    # Window chrome
    "minimize": s('<line x1="5" y1="12" x2="19" y2="12"/>'),
    "maximize": s('<rect x="4" y="4" width="16" height="16" rx="2"/>'),
    "restore": s(
        '<rect x="3" y="7" width="13" height="13" rx="1"/>'
        '<polyline points="7 7 7 3 21 3 21 17 17 17"/>'
    ),
    "pin": s(
        '<line x1="12" y1="17" x2="12" y2="22"/>'
        '<path d="M5 17h14v-1.76a2 2 0 0 0-1.11-1.79l-1.78-.9A2 2 0 0 1 15 10.76V6h1'
        "a2 2 0 0 0 0-4H8a2 2 0 0 0 0 4h1v4.76a2 2 0 0 1-1.11 1.79l-1.78.9"
        'A2 2 0 0 0 5 15.24Z"/>'
    ),
    "unpin": s(
        '<line x1="2" y1="2" x2="22" y2="22"/>'
        '<line x1="12" y1="17" x2="12" y2="22"/>'
        '<path d="M9 9v1.76a2 2 0 0 1-1.11 1.79l-1.78.9A2 2 0 0 0 5 15.24V17h14v-1.76'
        "a2 2 0 0 0-1.11-1.79l-1.78-.9A2 2 0 0 1 15 10.76V6h1"
        'a2 2 0 0 0 0-4H8"/>'
    ),
    # Navigation
    "chevron-left": s('<polyline points="15 18 9 12 15 6"/>'),
    "chevron-right": s('<polyline points="9 18 15 12 9 6"/>'),
    "chevron-up": s('<polyline points="18 15 12 9 6 15"/>'),
    "chevron-down": s('<polyline points="6 9 12 15 18 9"/>'),
    "arrow-left": s(
        '<line x1="19" y1="12" x2="5" y2="12"/>'
        '<polyline points="12 19 5 12 12 5"/>'
    ),
    "arrow-right": s(
        '<line x1="5" y1="12" x2="19" y2="12"/>'
        '<polyline points="12 5 19 12 12 19"/>'
    ),
    "arrow-up": s(
        '<line x1="12" y1="19" x2="12" y2="5"/>'
        '<polyline points="5 12 12 5 19 12"/>'
    ),
    "arrow-down": s(
        '<line x1="12" y1="5" x2="12" y2="19"/>'
        '<polyline points="19 12 12 19 5 12"/>'
    ),
    "home": s(
        '<path d="M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/>'
        '<polyline points="9 22 9 12 15 12 15 22"/>'
    ),
    "back": s(
        '<line x1="19" y1="12" x2="5" y2="12"/>'
        '<polyline points="12 19 5 12 12 5"/>'
    ),
    "forward": s(
        '<line x1="5" y1="12" x2="19" y2="12"/>'
        '<polyline points="12 5 19 12 12 19"/>'
    ),
    # Data operations
    "search": s(
        '<circle cx="11" cy="11" r="8"/>'
        '<line x1="21" y1="21" x2="16.65" y2="16.65"/>'
    ),
    "filter": s('<polygon points="22 3 2 3 10 12.46 10 19 14 21 14 12.46 22 3"/>'),
    "sort": s(
        '<line x1="12" y1="5" x2="12" y2="19"/>'
        '<polyline points="19 12 12 19 5 12"/>'
    ),
    "sort-asc": s(
        '<line x1="12" y1="19" x2="12" y2="5"/>'
        '<polyline points="5 12 12 5 19 12"/>'
    ),
    "sort-desc": s(
        '<line x1="12" y1="5" x2="12" y2="19"/>'
        '<polyline points="19 12 12 19 5 12"/>'
    ),
    # CRUD
    "add": s(
        '<line x1="12" y1="5" x2="12" y2="19"/>'
        '<line x1="5" y1="12" x2="19" y2="12"/>'
    ),
    "remove": s('<line x1="5" y1="12" x2="19" y2="12"/>'),
    "edit": s(
        '<path d="M11 4H4a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2v-7"/>'
        '<path d="M18.5 2.5a2.121 2.121 0 0 1 3 3L12 15l-4 1 1-4 9.5-9.5z"/>'
    ),
    "delete": s(
        '<polyline points="3 6 5 6 21 6"/>'
        '<path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4'
        'a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"/>'
    ),
    "copy": s(
        '<rect x="9" y="9" width="13" height="13" rx="2" ry="2"/>'
        '<path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1"/>'
    ),
    "paste": s(
        '<path d="M16 4h2a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2H6a2 2 0 0 1-2-2V6'
        'a2 2 0 0 1 2-2h2"/>'
        '<rect x="8" y="2" width="8" height="4" rx="1" ry="1"/>'
    ),
    "cut": s(
        '<circle cx="6" cy="6" r="3"/><circle cx="6" cy="18" r="3"/>'
        '<line x1="20" y1="4" x2="8.12" y2="15.88"/>'
        '<line x1="14.47" y1="14.48" x2="20" y2="20"/>'
        '<line x1="8.12" y1="8.12" x2="12" y2="12"/>'
    ),
    # Document
    "save": s(
        '<path d="M19 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11l5 5v11'
        'a2 2 0 0 1-2 2z"/>'
        '<polyline points="17 21 17 13 7 13 7 21"/>'
        '<polyline points="7 3 7 8 15 8"/>'
    ),
    "save-as": s(
        '<path d="M19 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11l5 5v11'
        'a2 2 0 0 1-2 2z"/>'
        '<polyline points="17 21 17 13 7 13 7 21"/>'
        '<line x1="12" y1="7" x2="12" y2="3"/>'
        '<line x1="10" y1="5" x2="14" y2="5"/>'
    ),
    "open": s(
        '<path d="M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5'
        'a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z"/>'
    ),
    "new-file": s(
        '<path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12'
        'a2 2 0 0 0 2-2V8z"/>'
        '<polyline points="14 2 14 8 20 8"/>'
        '<line x1="12" y1="18" x2="12" y2="12"/>'
        '<line x1="9" y1="15" x2="15" y2="15"/>'
    ),
    "new-folder": s(
        '<path d="M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5'
        'a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z"/>'
        '<line x1="12" y1="11" x2="12" y2="17"/>'
        '<line x1="9" y1="14" x2="15" y2="14"/>'
    ),
    # Viewport
    "zoom-in": s(
        '<circle cx="11" cy="11" r="8"/>'
        '<line x1="21" y1="21" x2="16.65" y2="16.65"/>'
        '<line x1="11" y1="8" x2="11" y2="14"/>'
        '<line x1="8" y1="11" x2="14" y2="11"/>'
    ),
    "zoom-out": s(
        '<circle cx="11" cy="11" r="8"/>'
        '<line x1="21" y1="21" x2="16.65" y2="16.65"/>'
        '<line x1="8" y1="11" x2="14" y2="11"/>'
    ),
    "zoom-fit": s(
        '<path d="M15 3h6v6"/><path d="M9 21H3v-6"/>'
        '<path d="M21 3l-7 7"/><path d="M3 21l7-7"/>'
    ),
    "fullscreen": s(
        '<path d="M8 3H5a2 2 0 0 0-2 2v3m18 0V5a2 2 0 0 0-2-2h-3'
        'm0 18h3a2 2 0 0 0 2-2v-3M3 16v3a2 2 0 0 0 2 2h3"/>'
    ),
    "rotate-left": s(
        '<polyline points="1 4 1 10 7 10"/>'
        '<path d="M3.51 15a9 9 0 1 0 2.13-9.36L1 10"/>'
    ),
    "rotate-right": s(
        '<polyline points="23 4 23 10 17 10"/>'
        '<path d="M20.49 15a9 9 0 1 1-2.13-9.36L23 10"/>'
    ),
    # Status
    "info": s(
        '<circle cx="12" cy="12" r="10"/>'
        '<line x1="12" y1="16" x2="12" y2="12"/>'
        '<line x1="12" y1="8" x2="12.01" y2="8"/>'
    ),
    "warning": s(
        '<path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94'
        'a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z"/>'
        '<line x1="12" y1="9" x2="12" y2="13"/>'
        '<line x1="12" y1="17" x2="12.01" y2="17"/>'
    ),
    "error": s(
        '<circle cx="12" cy="12" r="10"/>'
        '<line x1="15" y1="9" x2="9" y2="15"/>'
        '<line x1="9" y1="9" x2="15" y2="15"/>'
    ),
    "success": s(
        '<path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"/>'
        '<polyline points="22 4 12 14.01 9 11.01"/>'
    ),
    "help": s(
        '<circle cx="12" cy="12" r="10"/>'
        '<path d="M9.09 9a3 3 0 0 1 5.83 1c0 2-3 3-3 3"/>'
        '<line x1="12" y1="17" x2="12.01" y2="17"/>'
    ),
    "question": s(
        '<circle cx="12" cy="12" r="10"/>'
        '<path d="M9.09 9a3 3 0 0 1 5.83 1c0 2-3 3-3 3"/>'
        '<line x1="12" y1="17" x2="12.01" y2="17"/>'
    ),
    # Misc
    "settings": s(
        '<circle cx="12" cy="12" r="3"/>'
        '<path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83'
        " 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33"
        " 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09"
        "A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06"
        " a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06"
        "A1.65 1.65 0 0 0 4.68 15a1.65 1.65 0 0 0-1.51-1H3"
        " a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9"
        " a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83"
        " 2 2 0 0 1 2.83 0l.06.06A1.65 1.65 0 0 0 9 4.68"
        " a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09"
        " a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06"
        " a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06"
        " a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21"
        ' a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"/>'
    ),
    "menu": s(
        '<line x1="3" y1="12" x2="21" y2="12"/>'
        '<line x1="3" y1="6" x2="21" y2="6"/>'
        '<line x1="3" y1="18" x2="21" y2="18"/>'
    ),
    "more-horizontal": s(
        '<circle cx="12" cy="12" r="1"/>'
        '<circle cx="19" cy="12" r="1"/>'
        '<circle cx="5" cy="12" r="1"/>'
    ),
    "more-vertical": s(
        '<circle cx="12" cy="12" r="1"/>'
        '<circle cx="12" cy="5" r="1"/>'
        '<circle cx="12" cy="19" r="1"/>'
    ),
    "cross": s(
        '<line x1="18" y1="6" x2="6" y2="18"/>'
        '<line x1="6" y1="6" x2="18" y2="18"/>'
    ),
    "refresh": s(
        '<polyline points="23 4 23 10 17 10"/>'
        '<polyline points="1 20 1 14 7 14"/>'
        '<path d="M3.51 9a9 9 0 0 1 14.85-3.36L23 10'
        'M1 14l4.64 4.36A9 9 0 0 0 20.49 15"/>'
    ),
    "download": s(
        '<path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/>'
        '<polyline points="7 10 12 15 17 10"/>'
        '<line x1="12" y1="15" x2="12" y2="3"/>'
    ),
    "upload": s(
        '<path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/>'
        '<polyline points="17 8 12 3 7 8"/>'
        '<line x1="12" y1="3" x2="12" y2="15"/>'
    ),
    "eye": s(
        '<path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"/>'
        '<circle cx="12" cy="12" r="3"/>'
    ),
    "eye-off": s(
        '<path d="M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8'
        'a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4'
        'c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07'
        'a3 3 0 1 1-4.24-4.24"/>'
        '<line x1="1" y1="1" x2="23" y2="23"/>'
    ),
    "lock": s(
        '<rect x="3" y="11" width="18" height="11" rx="2" ry="2"/>'
        '<path d="M7 11V7a5 5 0 0 1 10 0v4"/>'
    ),
    "unlock": s(
        '<rect x="3" y="11" width="18" height="11" rx="2" ry="2"/>'
        '<path d="M7 11V7a5 5 0 0 1 9.9-1"/>'
    ),
}


def main():
    os.makedirs(ICONS_DIR, exist_ok=True)
    created = 0
    skipped = 0
    for name, content in ICONS.items():
        path = os.path.join(ICONS_DIR, f"{name}.svg")
        if os.path.exists(path):
            skipped += 1
            continue
        with open(path, "w", encoding="utf-8") as f:
            f.write(content)
        created += 1

    print(f"Created {created} SVG icons, skipped {skipped} existing")


if __name__ == "__main__":
    main()

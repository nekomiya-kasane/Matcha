#!/usr/bin/env python3
"""
Restructure Matcha_Design_System_Specification.md per doc_plan.md.

Strategy:
1. Parse the original spec into named blocks by Part/Chapter boundaries
2. Reassemble blocks in the new 7-pillar Part I order
3. Inject pillar section headers (## I.1, ## I.2, etc.)
4. Renumber section headers within moved blocks
5. Write the result

Run from: Gui/ directory
  python Matcha/docs/restructure_spec.py
"""
import re, shutil
from pathlib import Path

SPEC = Path(r"c:\Users\Nekomiya\Downloads\Gui\Matcha\docs\Matcha_Design_System_Specification.md")

def read_spec():
    return SPEC.read_text(encoding="utf-8")

def write_spec(content: str):
    SPEC.write_text(content, encoding="utf-8")
    print(f"Written {len(content.splitlines())} lines to {SPEC.name}")

def extract_blocks(lines: list[str]) -> dict[str, list[str]]:
    """Split the document into named blocks at # and ## boundaries."""
    blocks = {}
    current_name = "PREAMBLE"
    current_lines = []

    # Markers we split on (line must start with these)
    markers = [
        (r"^# Part ", "part"),
        (r"^## Chapter \d", "chapter"),
        (r"^# Appendices", "appendices"),
        (r"^## Appendix ", "appendix"),
    ]

    for line in lines:
        matched = False
        for pattern, kind in markers:
            if re.match(pattern, line):
                # Save current block
                if current_lines:
                    blocks[current_name] = current_lines
                # Start new block
                if kind == "part":
                    m = re.search(r"Part (\S+)", line)
                    current_name = f"PART_{m.group(1).rstrip('.')}" if m else line.strip()
                elif kind == "chapter":
                    m = re.search(r"Chapter (\d+)", line)
                    current_name = f"CH_{m.group(1)}" if m else line.strip()
                elif kind == "appendices":
                    current_name = "APPENDICES_HEADER"
                elif kind == "appendix":
                    m = re.search(r"Appendix (\w)", line)
                    current_name = f"APP_{m.group(1)}" if m else line.strip()
                current_lines = [line]
                matched = True
                break
        if not matched:
            current_lines.append(line)

    if current_lines:
        blocks[current_name] = current_lines

    return blocks

def renumber_sections(lines: list[str], old_ch: int, new_prefix: str, depth_increase: int = 0) -> list[str]:
    """Renumber ### old_ch.X to (### + depth_increase) new_prefix.X"""
    result = []
    old_pat = re.compile(rf"^(#{{{3}}}) {old_ch}\.(\d+)")
    old_sub_pat = re.compile(rf"^(#{{{4}}}) {old_ch}\.(\d+)\.(\d+)")
    extra_hashes = "#" * depth_increase

    for line in lines:
        # Sub-sections first (#### X.Y.Z)
        m = old_sub_pat.match(line)
        if m:
            result.append(f"{'####'}{extra_hashes} {new_prefix}.{m.group(2)}.{m.group(3)}" + line[m.end():] + "\n" if not line.endswith("\n") else f"{'####'}{extra_hashes} {new_prefix}.{m.group(2)}.{m.group(3)}" + line[m.end():])
            continue
        # Sections (### X.Y)
        m = old_pat.match(line)
        if m:
            result.append(f"{'###'}{extra_hashes} {new_prefix}.{m.group(2)}" + line[m.end():])
            continue
        result.append(line)
    return result

def build_new_document(blocks: dict[str, list[str]]) -> str:
    """Assemble the new document from extracted blocks."""
    parts = []

    # --- Preamble (title, TOC placeholder) ---
    # Keep original preamble but replace TOC
    preamble = blocks.get("PREAMBLE", [])
    # Find TOC start/end
    toc_start = None
    toc_end = None
    for i, line in enumerate(preamble):
        if line.strip() == "## Table of Contents":
            toc_start = i
        if toc_start is not None and line.startswith("---") and i > toc_start + 2:
            toc_end = i
            break

    # Write preamble up to TOC
    if toc_start is not None:
        parts.extend(preamble[:toc_start])
    else:
        parts.extend(preamble)

    # Write new minimal TOC (will be regenerated later)
    parts.append("## Table of Contents\n")
    parts.append("\n")
    parts.append("> **TOC will be regenerated after all content is finalized.**\n")
    parts.append("> See `doc_plan.md` for the complete 7-pillar structure.\n")
    parts.append("\n")

    # Write separator + rest of preamble after TOC
    if toc_end is not None:
        parts.extend(preamble[toc_end:])

    # --- Part 0 (unchanged) ---
    parts.extend(blocks.get("PART_0", []))
    parts.extend(blocks.get("CH_0", []))

    # --- Part I: Design Language Specification ---
    parts.append("\n---\n\n")
    parts.append("# Part I -- Design Language Specification\n")
    parts.append("\n")
    parts.append("> 🆕 **Post-v1 structural reorganization.** Part I is the designer's complete expression of the\n")
    parts.append("> Matcha design system. It defines all visual, typographic, spatial, stylistic, interactive, and\n")
    parts.append("> motion specifications needed to implement the UI from scratch.\n")
    parts.append(">\n")
    parts.append("> **7-pillar structure**: I.1 Foundations & Principles · I.2 Tokens · I.3 Typography ·\n")
    parts.append("> I.4 Style · I.5 Widgets · I.6 Layout · I.7 Interaction · I.8 Motion\n")
    parts.append(">\n")
    parts.append("> Later Parts (II–VIII) provide architecture and implementation details that realize\n")
    parts.append("> Part I's design intent. Part I cross-references those Parts where appropriate.\n")
    parts.append("\n")

    # I.1 Foundations & Principles (Ch 1)
    parts.append("## I.1 Foundations & Principles\n\n")
    ch1 = blocks.get("CH_1", [])
    # Demote ## Chapter -> ### Chapter
    ch1_demoted = [line.replace("## Chapter 1.", "### Chapter 1.") if line.startswith("## Chapter 1.") else line for line in ch1]
    # Renumber ### 1.X -> #### 1.X
    ch1_demoted = renumber_sections(ch1_demoted, 1, "1", depth_increase=1)
    parts.extend(ch1_demoted)

    # I.2 Tokens (Ch 2 Color + Ch 4 Spatial + Ch 16 Icon + Ch 17 Cursor)
    parts.append("\n## I.2 Tokens\n\n")
    parts.append("> Visual primitive definitions: color, spatial, icon design language, cursor.\n\n")

    # Ch 2 Color
    ch2 = blocks.get("CH_2", [])
    ch2 = [line.replace("## Chapter 2.", "### Chapter 2.") if line.startswith("## Chapter 2.") else line for line in ch2]
    ch2 = renumber_sections(ch2, 2, "2", depth_increase=1)
    parts.extend(ch2)

    # Ch 4 Spatial
    ch4 = blocks.get("CH_4", [])
    ch4 = [line.replace("## Chapter 4.", "### Chapter 4.") if line.startswith("## Chapter 4.") else line for line in ch4]
    ch4 = renumber_sections(ch4, 4, "4", depth_increase=1)
    parts.extend(ch4)

    # Ch 16 Icon Design Language (from old Part V)
    ch16 = blocks.get("CH_16", [])
    ch16 = [line.replace("## Chapter 16. Icon System", "### Chapter 6. Icon Design Language") if "Chapter 16. Icon System" in line else line for line in ch16]
    ch16 = renumber_sections(ch16, 16, "6", depth_increase=1)
    parts.extend(ch16)

    # Ch 17 Cursor (from old Part V)
    ch17 = blocks.get("CH_17", [])
    ch17 = [line.replace("## Chapter 17. Cursor System", "### Chapter 7. Cursor Design") if "Chapter 17. Cursor System" in line else line for line in ch17]
    ch17 = renumber_sections(ch17, 17, "7", depth_increase=1)
    parts.extend(ch17)

    # I.3 Typography (Ch 3)
    parts.append("\n## I.3 Typography\n\n")
    ch3 = blocks.get("CH_3", [])
    ch3 = [line.replace("## Chapter 3.", "### Chapter 3.") if line.startswith("## Chapter 3.") else line for line in ch3]
    ch3 = renumber_sections(ch3, 3, "3", depth_increase=1)
    parts.extend(ch3)

    # I.4 Style (Ch 9 Declarative Style Architecture from old Part III)
    parts.append("\n## I.4 Style\n\n")
    parts.append("> Token → Widget bridge: declarative style architecture, variant patterns, state matrices.\n")
    parts.append("> Implementation details (Resolve(), BuildDefaultVariants): see Part III.\n\n")
    ch9 = blocks.get("CH_9", [])
    ch9 = [line.replace("## Chapter 9. Declarative Style Architecture", "### Chapter 4. Declarative Style Architecture") if "Chapter 9." in line else line for line in ch9]
    ch9 = renumber_sections(ch9, 9, "4", depth_increase=1)
    parts.extend(ch9)

    # I.5 Widgets (Ch 10 Per-Widget from old Part III)
    parts.append("\n## I.5 Widgets\n\n")
    parts.append("> 46 components with full visual and behavioral specifications.\n")
    parts.append("> Each widget follows a 12-section template (Synopsis · Anatomy 🆕 · Theme ·\n")
    parts.append("> Variant×State · FSM 🆕 · Notifications · API · Animation · Math · Keyboard 🆕 · A11y · Usage 🆕).\n\n")
    ch10 = blocks.get("CH_10", [])
    ch10 = [line.replace("## Chapter 10. Per-Widget Component Specification", "### Chapter 5. Per-Widget Component Specification") if "Chapter 10." in line else line for line in ch10]
    ch10 = renumber_sections(ch10, 10, "5", depth_increase=1)
    parts.extend(ch10)

    # I.6 Layout 🆕 (placeholder for new content)
    parts.append("\n## I.6 Layout 🆕\n\n")
    parts.append("> 🆕 Post-v1 supplement.\n")
    parts.append("> Layout algorithms, composition templates, responsive rules, loading/empty/error state templates.\n")
    parts.append("> Content to be added in Step 2.\n\n")

    # I.7 Interaction (Ch 18 A11y + Ch 19 i18n from old Part VI + new patterns)
    parts.append("\n## I.7 Interaction\n\n")
    parts.append("> 🆕 Cross-component interaction patterns, accessibility design rules, and i18n design.\n\n")

    # New interaction sections placeholder
    parts.append("### 7.1 Selection Model 🆕\n\n> 🆕 Post-v1 supplement. Content to be added.\n\n")
    parts.append("### 7.2 Form Validation 🆕\n\n> 🆕 Post-v1 supplement. Content to be added.\n\n")
    parts.append("### 7.3 Scroll & Virtualization 🆕\n\n> 🆕 Post-v1 supplement. Content to be added.\n\n")
    parts.append("### 7.4 Popup Positioning 🆕\n\n> 🆕 Post-v1 supplement. Content to be added.\n\n")
    parts.append("### 7.5 Text Overflow & Truncation 🆕\n\n> 🆕 Post-v1 supplement. Content to be added.\n\n")
    parts.append("### 7.6 Context Menu Composition 🆕\n\n> 🆕 Post-v1 supplement. Content to be added.\n\n")
    parts.append("### 7.7 Notification Stacking 🆕\n\n> 🆕 Post-v1 supplement. Content to be added.\n\n")
    parts.append("### 7.8 Drag & Drop Design 🆕\n\n> 🆕 Post-v1 supplement. Content to be added.\n\n")

    # Ch 18 A11y -> I.7 §7.9
    ch18 = blocks.get("CH_18", [])
    ch18 = [line.replace("## Chapter 18. Accessibility Infrastructure", "### 7.9 Accessibility Design") if "Chapter 18." in line else line for line in ch18]
    ch18 = renumber_sections(ch18, 18, "7.9", depth_increase=1)
    parts.extend(ch18)

    # Ch 19 i18n -> I.7 §7.10
    ch19 = blocks.get("CH_19", [])
    ch19 = [line.replace("## Chapter 19. Internationalization", "### 7.10 Internationalization Design") if "Chapter 19." in line else line for line in ch19]
    ch19 = renumber_sections(ch19, 19, "7.10", depth_increase=1)
    parts.extend(ch19)

    # Feedback/Signifier/CogLoad placeholders
    parts.append("\n### 7.11 Feedback & System Status 🆕\n\n> 🆕 Post-v1 supplement. Content to be added.\n\n")
    parts.append("### 7.12 Signifier Design 🆕\n\n> 🆕 Post-v1 supplement. Content to be added.\n\n")
    parts.append("### 7.13 Cognitive Load Thresholds 🆕\n\n> 🆕 Post-v1 supplement. Content to be added.\n\n")

    # I.8 Motion (Ch 5 from old Part I + new timing/choreography)
    parts.append("\n## I.8 Motion\n\n")
    parts.append("> Time-based design language: duration tokens, easing curves, spring dynamics,\n")
    parts.append("> interaction timing, choreography, gesture-driven motion.\n\n")
    ch5 = blocks.get("CH_5", [])
    ch5 = [line.replace("## Chapter 5. Motion System", "### Chapter 5. Motion & Timing System") if "Chapter 5." in line else line for line in ch5]
    ch5 = renumber_sections(ch5, 5, "8", depth_increase=1)
    parts.extend(ch5)

    # New motion sections placeholders
    parts.append("\n### 8.7 Interaction Timing Tokens 🆕\n\n> 🆕 Post-v1 supplement. Content to be added.\n\n")
    parts.append("### 8.8 Choreography 🆕\n\n> 🆕 Post-v1 supplement. Content to be added.\n\n")
    parts.append("### 8.9 Gesture-Driven Motion 🆕\n\n> 🆕 Post-v1 supplement. Content to be added.\n\n")

    # ===== Parts II+ (architecture/implementation) =====

    # Part II: Theme Engine (Ch 6, 7, 8 -> Ch 9, 10, 11)
    parts.append("\n---\n---\n\n")
    parts.append("# Part II -- Theme Engine\n\n")
    parts.append("> Chapters 9-11. The runtime infrastructure that resolves Part I's token definitions\n")
    parts.append("> to concrete values. Implements IThemeService, JSON configuration, NyanTheme.\n\n")

    for old_ch, new_ch in [(6, 9), (7, 10), (8, 11)]:
        blk = blocks.get(f"CH_{old_ch}", [])
        blk = [re.sub(rf"## Chapter {old_ch}\.", f"## Chapter {new_ch}.", line) if f"Chapter {old_ch}." in line else line for line in blk]
        blk = renumber_sections(blk, old_ch, str(new_ch))
        parts.extend(blk)

    # Part III: Component Style Architecture (Ch 11 -> Ch 14)
    parts.append("\n---\n\n")
    parts.append("# Part III -- Component Style Architecture\n\n")
    parts.append("> Chapters 13-14. Implements Part I §4 Style's declarative resolution.\n\n")

    # Ch 11 WidgetKind Registry -> Ch 14
    ch11 = blocks.get("CH_11", [])
    ch11 = [line.replace("## Chapter 11.", "## Chapter 14.") if "Chapter 11." in line else line for line in ch11]
    ch11 = renumber_sections(ch11, 11, "14")
    parts.extend(ch11)

    # Part IV: Animation Engine (Ch 12-15 -> Ch 15-18)
    parts.append("\n---\n---\n\n")
    parts.append("# Part IV -- Animation Engine\n\n")
    parts.append("> Chapters 15-18. Implements Part I §8 Motion's physics and choreography.\n\n")

    for old_ch, new_ch in [(12, 15), (13, 16), (14, 17), (15, 18)]:
        blk = blocks.get(f"CH_{old_ch}", [])
        blk = [re.sub(rf"## Chapter {old_ch}\.", f"## Chapter {new_ch}.", line) if f"Chapter {old_ch}." in line else line for line in blk]
        blk = renumber_sections(blk, old_ch, str(new_ch))
        parts.extend(blk)

    # Part V: Accessibility & i18n Infrastructure (implementation parts)
    # Note: design rules moved to Part I §7. This Part keeps FocusManager, A11yAudit etc.
    # For now, placeholder since the content was moved to I.7
    parts.append("\n---\n---\n\n")
    parts.append("# Part V -- Accessibility & i18n Infrastructure 🆕\n\n")
    parts.append("> Implementation details for Part I §7.9-7.10.\n")
    parts.append("> FocusManager, A11yAudit, focus trap, mnemonic system architecture.\n")
    parts.append("> Note: design rules are now in Part I §7. This Part covers only implementation architecture.\n\n")

    # Part VI: Dynamic Injection (Ch 20-22 -> Ch 22-24)
    parts.append("\n---\n---\n\n")
    parts.append("# Part VI -- Dynamic Injection\n\n")
    parts.append("> Chapters 22-24.\n\n")

    for old_ch, new_ch in [(20, 22), (21, 23), (22, 24)]:
        blk = blocks.get(f"CH_{old_ch}", [])
        blk = [re.sub(rf"## Chapter {old_ch}\.", f"## Chapter {new_ch}.", line) if f"Chapter {old_ch}." in line else line for line in blk]
        blk = renumber_sections(blk, old_ch, str(new_ch))
        parts.extend(blk)

    # Part VII: Testing & Validation (Ch 23-25 -> Ch 25-27)
    parts.append("\n---\n---\n\n")
    parts.append("# Part VII -- Testing & Validation\n\n")
    parts.append("> Chapters 25-27.\n\n")

    for old_ch, new_ch in [(23, 25), (24, 26), (25, 27)]:
        blk = blocks.get(f"CH_{old_ch}", [])
        blk = [re.sub(rf"## Chapter {old_ch}\.", f"## Chapter {new_ch}.", line) if f"Chapter {old_ch}." in line else line for line in blk]
        blk = renumber_sections(blk, old_ch, str(new_ch))
        parts.extend(blk)

    # Part VIII: UI Architecture (Ch 26-29 -> Ch 28-31)
    parts.append("\n---\n---\n\n")
    # Get the Part IX block which has the "why architecture" preamble
    part_ix = blocks.get("PART_IX", [])
    # Replace title
    parts.append("# Part VIII -- UI Architecture\n\n")
    # Include the preamble content (skip the old # Part IX line)
    for line in part_ix[1:]:
        parts.append(line)

    for old_ch, new_ch in [(26, 28), (27, 29), (28, 30), (29, 31)]:
        blk = blocks.get(f"CH_{old_ch}", [])
        blk = [re.sub(rf"## Chapter {old_ch}\.", f"## Chapter {new_ch}.", line) if f"Chapter {old_ch}." in line else line for line in blk]
        blk = renumber_sections(blk, old_ch, str(new_ch))
        parts.extend(blk)

    # Part IX: Roadmap (Ch 30 -> Ch 32)
    parts.append("\n---\n---\n\n")
    parts.append("# Part IX -- Implementation Roadmap\n\n")

    ch30 = blocks.get("CH_30", [])
    ch30 = [line.replace("## Chapter 30.", "## Chapter 32.") if "Chapter 30." in line else line for line in ch30]
    ch30 = renumber_sections(ch30, 30, "32")
    parts.extend(ch30)

    # Appendices
    parts.append("\n---\n---\n\n")
    parts.extend(blocks.get("APPENDICES_HEADER", []))
    for app_key in ["APP_A", "APP_B", "APP_C", "APP_D", "APP_E"]:
        if app_key in blocks:
            parts.extend(blocks[app_key])

    return "".join(parts)


def main():
    print("Reading original spec...")
    content = read_spec()
    lines = content.splitlines(keepends=True)
    print(f"  {len(lines)} lines")

    # Backup
    shutil.copy2(SPEC, SPEC.with_suffix(".md.bak"))
    print(f"  Backup: {SPEC.with_suffix('.md.bak')}")

    print("\nExtracting blocks...")
    blocks = extract_blocks(lines)
    print(f"  Found {len(blocks)} blocks:")
    for name, blk_lines in blocks.items():
        print(f"    {name}: {len(blk_lines)} lines")

    print("\nReassembling document...")
    new_content = build_new_document(blocks)
    new_lines = len(new_content.splitlines())
    print(f"  Result: {new_lines} lines")

    print("\nWriting...")
    write_spec(new_content)
    print("\nDone!")


if __name__ == "__main__":
    main()

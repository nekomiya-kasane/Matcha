#!/usr/bin/env python3
"""
validate_tokens.py — Build-time palette JSON validator for Matcha.

Usage:
    python validate_tokens.py <schema.json> <palette1.json> [palette2.json ...]

Exit codes:
    0  All palettes valid.
    1  One or more palettes failed validation.
    2  Usage error or missing files.

Dependencies:
    Python 3.8+ standard library only (no pip packages).
    Uses a lightweight validation approach since jsonschema is not guaranteed
    to be available in all build environments.
"""

import json
import re
import sys
from pathlib import Path


# ---------------------------------------------------------------------------
# Lightweight JSON Schema validator (subset: required, properties, pattern,
# additionalProperties, $ref, type, minLength, enum).
# Full JSON Schema Draft 2020-12 is overkill; we only need structural checks.
# ---------------------------------------------------------------------------

_COLOR_RE = re.compile(r"^#([0-9A-Fa-f]{6}|[0-9A-Fa-f]{8})$")

# Neutral + special color token keys (must match DesignTokens.h ColorToken enum).
# Semantic hue tokens (5 hues x 10 steps = 50) are generated from colorSeeds.
REQUIRED_COLOR_KEYS = [
    # Neutral: Surface (5)
    "Surface", "SurfaceContainer", "SurfaceElevated", "SurfaceSunken", "Spotlight",
    # Neutral: Fill (4)
    "Fill", "FillHover", "FillActive", "FillMuted",
    # Neutral: Border (3)
    "BorderSubtle", "BorderDefault", "BorderStrong",
    # Neutral: Text (4)
    "TextPrimary", "TextSecondary", "TextTertiary", "TextDisabled",
    # Special (8)
    "OnAccent", "OnAccentSecondary", "Focus", "Selection", "Link", "Scrim", "Overlay", "Shadow", "Separator",
]

# Semantic hue token keys (5 hues x 10 steps = 50), generated from colorSeeds.
_HUE_SUFFIXES = ["Bg", "BgHover", "Border", "BorderHover", "Hover",
                 "", "Active", "TextHover", "Text", "TextActive"]
SEMANTIC_HUE_KEYS = []
for _hue in ("Primary", "Success", "Warning", "Error", "Info"):
    for _suf in _HUE_SUFFIXES:
        SEMANTIC_HUE_KEYS.append(_hue + _suf)

# All valid color keys (neutral + special + semantic)
ALL_COLOR_KEYS = REQUIRED_COLOR_KEYS + SEMANTIC_HUE_KEYS

REQUIRED_FONT_KEYS = ["family_win", "family_mac", "family_linux", "family_mono"]

VALID_COLOR_SEED_KEYS = {"Primary", "Success", "Warning", "Error", "Info"}


def validate_color(value: str, key: str) -> list[str]:
    """Validate a single color value."""
    errors = []
    if not isinstance(value, str):
        errors.append(f"  colors.{key}: expected string, got {type(value).__name__}")
    elif not _COLOR_RE.match(value):
        errors.append(f"  colors.{key}: invalid color format '{value}' (expected #RRGGBB or #AARRGGBB)")
    return errors


def validate_palette(palette_path: Path) -> list[str]:
    """Validate a single palette JSON file. Returns list of error strings."""
    errors = []

    try:
        with open(palette_path, "r", encoding="utf-8") as f:
            data = json.load(f)
    except json.JSONDecodeError as e:
        return [f"  JSON parse error: {e}"]
    except OSError as e:
        return [f"  File read error: {e}"]

    if not isinstance(data, dict):
        return ["  Root must be a JSON object"]

    # --- name ---
    name = data.get("name")
    if not isinstance(name, str) or len(name) == 0:
        errors.append("  'name' is required and must be a non-empty string")

    # --- extends (optional) ---
    if "extends" in data:
        ext = data["extends"]
        if not isinstance(ext, str) or len(ext) == 0:
            errors.append("  'extends' must be a non-empty string if present")

    # --- colors ---
    colors = data.get("colors")
    has_color_seeds = "colorSeeds" in data

    if colors is None and not has_color_seeds:
        errors.append("  'colors' object is required (unless 'colorSeeds' provides all semantic hues)")
    elif colors is not None:
        if not isinstance(colors, dict):
            errors.append("  'colors' must be a JSON object")
        else:
            # Check required neutral + special keys
            for key in REQUIRED_COLOR_KEYS:
                if key not in colors:
                    errors.append(f"  colors: missing required key '{key}'")
                else:
                    errors.extend(validate_color(colors[key], key))

            # Semantic hue keys: required only if NOT provided by colorSeeds
            if not has_color_seeds:
                for key in SEMANTIC_HUE_KEYS:
                    if key not in colors:
                        errors.append(f"  colors: missing required key '{key}' (no colorSeeds to generate it)")
                    else:
                        errors.extend(validate_color(colors[key], key))

            # Check for unknown keys
            allowed = set(ALL_COLOR_KEYS)
            for key in colors:
                if key not in allowed:
                    errors.append(f"  colors: unknown key '{key}' (not in ColorToken enum)")

    # --- colorSeeds (optional) ---
    if has_color_seeds:
        seeds = data["colorSeeds"]
        if not isinstance(seeds, dict):
            errors.append("  'colorSeeds' must be a JSON object")
        else:
            for key in seeds:
                if key not in VALID_COLOR_SEED_KEYS:
                    errors.append(f"  colorSeeds: unknown key '{key}' (expected: {VALID_COLOR_SEED_KEYS})")
                else:
                    errors.extend(validate_color(seeds[key], f"colorSeeds.{key}"))

    # --- colorOverrides (optional) ---
    if "colorOverrides" in data:
        overrides = data["colorOverrides"]
        if not isinstance(overrides, dict):
            errors.append("  'colorOverrides' must be a JSON object")
        else:
            allowed = set(ALL_COLOR_KEYS)
            for key, value in overrides.items():
                if key not in allowed:
                    errors.append(f"  colorOverrides: unknown key '{key}'")
                else:
                    errors.extend(validate_color(value, f"colorOverrides.{key}"))

    # --- font ---
    font = data.get("font")
    if font is None:
        errors.append("  'font' object is required")
    elif not isinstance(font, dict):
        errors.append("  'font' must be a JSON object")
    else:
        for key in REQUIRED_FONT_KEYS:
            if key not in font:
                errors.append(f"  font: missing required key '{key}'")
            elif not isinstance(font[key], str) or len(font[key]) == 0:
                errors.append(f"  font.{key}: must be a non-empty string")

        allowed_font = set(REQUIRED_FONT_KEYS)
        for key in font:
            if key not in allowed_font:
                errors.append(f"  font: unknown key '{key}'")

    # --- top-level unknown keys ---
    allowed_top = {"name", "extends", "colors", "colorSeeds", "colorOverrides", "font", "fontScale", "fonts", "spring"}
    for key in data:
        if key not in allowed_top:
            errors.append(f"  unknown top-level key '{key}'")

    return errors


def main() -> int:
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <schema.json> <palette1.json> [palette2.json ...]", file=sys.stderr)
        print("  schema.json is accepted for CLI compatibility but not used at runtime.", file=sys.stderr)
        return 2

    # argv[1] is schema path (for CMake compatibility); we use built-in validation.
    palette_paths = [Path(p) for p in sys.argv[2:]]

    all_ok = True
    for path in palette_paths:
        if not path.exists():
            print(f"FAIL {path}: file not found", file=sys.stderr)
            all_ok = False
            continue

        errors = validate_palette(path)
        if errors:
            print(f"FAIL {path.name}:")
            for err in errors:
                print(err)
            all_ok = False
        else:
            print(f"OK   {path.name}")

    if all_ok:
        print(f"\nAll {len(palette_paths)} palette(s) valid.")
    else:
        print(f"\nValidation failed.", file=sys.stderr)

    return 0 if all_ok else 1


if __name__ == "__main__":
    sys.exit(main())

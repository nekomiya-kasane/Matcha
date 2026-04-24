#!/usr/bin/env python3
"""
validate_tokens.py — Build-time theme JSON validator for Matcha.

Usage:
    python validate_tokens.py <schema.json> <theme1.json> [theme2.json ...]

Exit codes:
    0  All themes valid.
    1  One or more themes failed validation.
    2  Usage error or missing files.

Dependencies:
    Python 3.8+ standard library only (no pip packages).
"""

import json
import re
import sys
from pathlib import Path

# ---------------------------------------------------------------------------
# Constants based on the design specification
# ---------------------------------------------------------------------------

# Allowed top-level keys
ALLOWED_TOP_KEYS = {"name", "extends", "colorSeeds", "colors", "fonts", "spring", "fontScale"}

# Required keys in colorSeeds (matches ColorToken seeds per spec §2.1.1)
COLOR_SEED_KEYS = {
    "colorPrimaryBase",
    "colorPrimaryNavBase",
    "colorSuccessBase",
    "colorWarningBase",
    "colorErrorBase",
    "colorTextBase",
    "colorBgBase",
}

# Required keys in fonts (matches FontRole seeds per spec §3.1)
# Note: fontFamily is NOT a seed token; it is determined by platform at runtime.
FONT_KEYS = {
    "fontWeightRegular",
    "fontWeightMedium",
    "fontWeightBold",
    "fontItalic",
    "fontLineHeight",
    "fontLetterSpacing",
    "fontSizeBase",
}

# Required keys in spring
SPRING_KEYS = {"mass", "stiffness", "damping"}

# Color hex pattern
COLOR_RE = re.compile(r"^#([0-9A-Fa-f]{6}|[0-9A-Fa-f]{8})$")


# ---------------------------------------------------------------------------
# Validation helpers
# ---------------------------------------------------------------------------

def validate_color(value, key_path):
    """Validate a single color value."""
    errors = []
    if not isinstance(value, str):
        errors.append(f"  {key_path}: expected string, got {type(value).__name__}")
    elif not COLOR_RE.match(value):
        errors.append(f"  {key_path}: invalid color format '{value}' (expected #RRGGBB or #AARRGGBB)")
    return errors


def validate_number(value, key_path, min_val=None, max_val=None):
    """Validate a number value (int or float)."""
    errors = []
    if not isinstance(value, (int, float)):
        errors.append(f"  {key_path}: expected number, got {type(value).__name__}")
    else:
        if min_val is not None and value < min_val:
            errors.append(f"  {key_path}: value {value} is below minimum {min_val}")
        if max_val is not None and value > max_val:
            errors.append(f"  {key_path}: value {value} exceeds maximum {max_val}")
    return errors


def validate_string(value, key_path):
    """Validate a string value."""
    errors = []
    if not isinstance(value, str):
        errors.append(f"  {key_path}: expected string, got {type(value).__name__}")
    elif len(value) == 0:
        errors.append(f"  {key_path}: string cannot be empty")
    return errors


def validate_boolean(value, key_path):
    """Validate a boolean value."""
    errors = []
    if not isinstance(value, bool):
        errors.append(f"  {key_path}: expected boolean, got {type(value).__name__}")
    return errors


def validate_color_seeds(obj, path_prefix):
    """Validate colorSeeds object."""
    errors = []
    if not isinstance(obj, dict):
        errors.append(f"  {path_prefix}: must be a JSON object")
        return errors

    # Check required keys
    for key in COLOR_SEED_KEYS:
        if key not in obj:
            errors.append(f"  {path_prefix}: missing required key '{key}'")
        else:
            errors.extend(validate_color(obj[key], f"{path_prefix}.{key}"))

    # Check for unknown keys
    for key in obj:
        if key not in COLOR_SEED_KEYS:
            errors.append(f"  {path_prefix}: unknown key '{key}'")
    return errors


def validate_colors(obj, path_prefix):
    """Validate optional colors object (per-token overrides)."""
    errors = []
    if obj is None:
        return errors
    if not isinstance(obj, dict):
        errors.append(f"  {path_prefix}: must be a JSON object")
        return errors

    # ColorToken names are lower camelCase (e.g., "colorPrimary", "focus")
    # We only check that keys are strings and values are valid colors.
    for key, value in obj.items():
        if not isinstance(key, str) or not key:
            errors.append(f"  {path_prefix}: invalid key (must be non-empty string)")
        else:
            errors.extend(validate_color(value, f"{path_prefix}.{key}"))
    return errors


def validate_fonts(obj, path_prefix):
    """Validate fonts object."""
    errors = []
    if not isinstance(obj, dict):
        errors.append(f"  {path_prefix}: must be a JSON object")
        return errors

    # Check required keys
    for key in FONT_KEYS:
        if key not in obj:
            errors.append(f"  {path_prefix}: missing required key '{key}'")
        else:
            if key == "fontItalic":
                errors.extend(validate_boolean(obj[key], f"{path_prefix}.{key}"))
            elif key in ("fontWeightRegular", "fontWeightMedium", "fontWeightBold"):
                errors.extend(validate_number(obj[key], f"{path_prefix}.{key}", 100, 900))
            elif key == "fontSizeBase":
                errors.extend(validate_number(obj[key], f"{path_prefix}.{key}", 6, 72))
            elif key in ("fontLineHeight", "fontLetterSpacing"):
                errors.extend(validate_number(obj[key], f"{path_prefix}.{key}"))
            else:
                # Should not happen, but defensive
                errors.append(f"  {path_prefix}: unexpected key '{key}'")

    # Check for unknown keys
    for key in obj:
        if key not in FONT_KEYS:
            errors.append(f"  {path_prefix}: unknown key '{key}'")
    return errors


def validate_spring(obj, path_prefix):
    """Validate spring object (optional)."""
    errors = []
    if obj is None:
        return errors
    if not isinstance(obj, dict):
        errors.append(f"  {path_prefix}: must be a JSON object")
        return errors

    for key in SPRING_KEYS:
        if key not in obj:
            errors.append(f"  {path_prefix}: missing required key '{key}'")
        else:
            errors.extend(validate_number(obj[key], f"{path_prefix}.{key}", 0.01 if key == "mass" else 0.0))

    for key in obj:
        if key not in SPRING_KEYS:
            errors.append(f"  {path_prefix}: unknown key '{key}'")
    return errors


def validate_font_scale(value, path_prefix):
    """Validate fontScale (optional)."""
    if value is None:
        return []
    return validate_number(value, path_prefix, 0.5, 3.0)


# ---------------------------------------------------------------------------
# Main validation function
# ---------------------------------------------------------------------------

def validate_theme(theme_path: Path) -> list[str]:
    """Validate a single theme JSON file. Returns list of error strings."""
    errors = []

    try:
        with open(theme_path, "r", encoding="utf-8") as f:
            data = json.load(f)
    except json.JSONDecodeError as e:
        return [f"  JSON parse error: {e}"]
    except OSError as e:
        return [f"  File read error: {e}"]

    if not isinstance(data, dict):
        return ["  Root must be a JSON object"]

    # --- name (required) ---
    name = data.get("name")
    if name is None:
        errors.append("  'name' is required")
    else:
        errors.extend(validate_string(name, "name"))

    # --- extends (optional) ---
    if "extends" in data:
        errors.extend(validate_string(data["extends"], "extends"))

    # --- colorSeeds (required) ---
    if "colorSeeds" not in data:
        errors.append("  'colorSeeds' object is required")
    else:
        errors.extend(validate_color_seeds(data["colorSeeds"], "colorSeeds"))

    # --- colors (optional) ---
    errors.extend(validate_colors(data.get("colors"), "colors"))

    # --- fonts (required) ---
    if "fonts" not in data:
        errors.append("  'fonts' object is required")
    else:
        errors.extend(validate_fonts(data["fonts"], "fonts"))

    # --- spring (optional) ---
    errors.extend(validate_spring(data.get("spring"), "spring"))

    # --- fontScale (optional) ---
    errors.extend(validate_font_scale(data.get("fontScale"), "fontScale"))

    # --- Check for unknown top-level keys ---
    for key in data:
        if key not in ALLOWED_TOP_KEYS:
            errors.append(f"  unknown top-level key '{key}'")

    return errors


def main() -> int:
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <schema.json> <theme1.json> [theme2.json ...]", file=sys.stderr)
        print("  schema.json is accepted for CLI compatibility but not used at runtime.", file=sys.stderr)
        return 2

    # argv[1] is schema path (ignored for validation)
    theme_paths = [Path(p) for p in sys.argv[2:]]

    all_ok = True
    for path in theme_paths:
        if not path.exists():
            print(f"FAIL {path}: file not found", file=sys.stderr)
            all_ok = False
            continue

        errors = validate_theme(path)
        if errors:
            print(f"FAIL {path.name}:")
            for err in errors:
                print(err)
            all_ok = False
        else:
            print(f"OK   {path.name}")

    if all_ok:
        print(f"\nAll {len(theme_paths)} theme(s) valid.")
    else:
        print(f"\nValidation failed.", file=sys.stderr)

    return 0 if all_ok else 1


if __name__ == "__main__":
    sys.exit(main())
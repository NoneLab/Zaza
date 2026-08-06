#!/usr/bin/env python3

from __future__ import annotations

import argparse
import datetime
import os
import re
import subprocess
import sys
from pathlib import Path


CPP_EXTENSIONS = {
    ".c",
    ".cc",
    ".cpp",
    ".cxx",
    ".h",
    ".hh",
    ".hpp",
    ".hxx",
}


DEFAULT_BRIEF = "TODO: Add a brief file description."


# Matches the generated header only when it appears at the beginning of a file.
#
# Supported form:
#
# // SPDX-License-Identifier: ...
#
# /**
#  * @file ...
#  * ...
#  */
#
FILE_HEADER_PATTERN = re.compile(
    r"\A"
    r"(?:"
    r"//[ \t]*SPDX-License-Identifier:[^\r\n]*"
    r"(?:\r?\n){1,2}"
    r")?"
    r"(?P<doxygen>"
    r"/\*\*"
    r".*?"
    r"^[ \t]*\*[ \t]*@file\b"
    r".*?"
    r"\*/"
    r")"
    r"(?:\r?\n){1,2}",
    re.DOTALL | re.MULTILINE,
)


BRIEF_PATTERN = re.compile(
    r"^[ \t]*\*[ \t]*@brief(?:[ \t]+(?P<brief>.*?))?[ \t]*$",
    re.MULTILINE,
)


def get_git_config(key: str) -> str | None:
    """Return a Git configuration value, or None if unavailable."""

    try:
        result = subprocess.run(
            ["git", "config", "--get", key],
            capture_output=True,
            text=True,
            check=False,
        )
    except OSError:
        return None

    value = result.stdout.strip()
    return value if value else None


def get_default_author_name() -> str:
    """Return the default author name."""

    return (
        get_git_config("user.name")
        or os.environ.get("USER")
        or os.environ.get("USERNAME")
        or "Unknown"
    )


def get_default_author_email() -> str:
    """Return the default author email address."""

    return get_git_config("user.email") or "unknown@example.com"


def format_author(name: str, email: str) -> str:
    """Format an author name and email address."""

    return f"{name} <{email}>"


def detect_newline(content: str) -> str:
    """Detect the newline sequence used by the file."""

    if "\r\n" in content[:4096]:
        return "\r\n"

    return "\n"


def extract_existing_brief(doxygen_header: str) -> str | None:
    """Extract the current Doxygen @brief value."""

    match = BRIEF_PATTERN.search(doxygen_header)

    if match is None:
        return None

    brief = match.group("brief")

    if brief is None:
        return None

    brief = brief.strip()
    return brief if brief else None


def generate_header(
    path: Path,
    brief: str,
    author_name: str,
    author_email: str,
    copyright_holder: str,
    license_id: str,
    newline: str,
) -> str:
    """Generate an SPDX and Doxygen source file header."""

    today = datetime.date.today()

    lines = [
        f"// SPDX-License-Identifier: {license_id}",
        "",
        "/**",
        f" * @file {path.name}",
        f" * @brief {brief}",
        " *",
        f" * @author {format_author(author_name, author_email)}",
        f" * @date {today.isoformat()}",
        " *",
        (
            f" * @copyright Copyright (c) "
            f"{today.year} {copyright_holder}"
        ),
        " */",
        "",
        "",
    ]

    return newline.join(lines)


def process_file(
    path: Path,
    brief: str | None,
    author_name: str,
    author_email: str,
    copyright_holder: str,
    license_id: str,
    dry_run: bool,
) -> str:
    """
    Add or update the generated header.

    Returns:
        "added", "updated", "unchanged", "skipped", or "error".
    """

    try:
        raw = path.read_bytes()
    except OSError as error:
        print(
            f"[ERROR] Failed to read {path}: {error}",
            file=sys.stderr,
        )
        return "error"

    utf8_bom = b"\xef\xbb\xbf"
    has_bom = raw.startswith(utf8_bom)

    if has_bom:
        raw = raw[len(utf8_bom):]

    try:
        content = raw.decode("utf-8")
    except UnicodeDecodeError:
        print(f"[SKIP] File is not UTF-8 encoded: {path}")
        return "skipped"

    newline = detect_newline(content)
    header_match = FILE_HEADER_PATTERN.match(content)

    if header_match is not None:
        current_doxygen_header = header_match.group("doxygen")
        current_brief = extract_existing_brief(current_doxygen_header)

        effective_brief = (
            brief
            if brief is not None
            else current_brief or DEFAULT_BRIEF
        )

        new_header = generate_header(
            path=path,
            brief=effective_brief,
            author_name=author_name,
            author_email=author_email,
            copyright_holder=copyright_holder,
            license_id=license_id,
            newline=newline,
        )

        updated_content = new_header + content[header_match.end():]
        action = "updated"

    else:
        beginning = content[:4096]

        # Avoid adding a second header if an unsupported/custom header exists.
        if (
            "@file" in beginning
            or "SPDX-License-Identifier:" in beginning
        ):
            print(
                "[SKIP] Existing header format is not recognized: "
                f"{path}"
            )
            return "skipped"

        effective_brief = brief or DEFAULT_BRIEF

        new_header = generate_header(
            path=path,
            brief=effective_brief,
            author_name=author_name,
            author_email=author_email,
            copyright_holder=copyright_holder,
            license_id=license_id,
            newline=newline,
        )

        updated_content = new_header + content
        action = "added"

    if updated_content == content:
        print(f"[UNCHANGED] File header is already current: {path}")
        return "unchanged"

    if dry_run:
        label = "update" if action == "updated" else "addition"

        print(f"[DRY-RUN] Planned header {label}: {path}")
        print(new_header, end="")
        return action

    output = updated_content.encode("utf-8")

    if has_bom:
        output = utf8_bom + output

    try:
        path.write_bytes(output)
    except OSError as error:
        print(
            f"[ERROR] Failed to write {path}: {error}",
            file=sys.stderr,
        )
        return "error"

    if action == "updated":
        print(f"[UPDATED] Replaced file header: {path}")
    else:
        print(f"[ADDED] Added file header: {path}")

    return action


def collect_cpp_files(target: Path, recursive: bool) -> list[Path]:
    """Collect supported C and C++ source files."""

    if target.is_file():
        if target.suffix.lower() in CPP_EXTENSIONS:
            return [target]

        return []

    if not target.is_dir():
        return []

    iterator = target.rglob("*") if recursive else target.glob("*")

    return sorted(
        path
        for path in iterator
        if path.is_file() and path.suffix.lower() in CPP_EXTENSIONS
    )


def main() -> int:
    """Run the command-line application."""

    default_name = get_default_author_name()
    default_email = get_default_author_email()

    parser = argparse.ArgumentParser(
        description=(
            "Add or update SPDX and Doxygen file headers "
            "in C and C++ source files."
        )
    )

    parser.add_argument(
        "target",
        type=Path,
        help="C/C++ source file or directory to process.",
    )

    parser.add_argument(
        "--author-name",
        default=default_name,
        help=(
            "Author name. "
            f"Default: value of git config user.name ({default_name})."
        ),
    )

    parser.add_argument(
        "--author-email",
        default=default_email,
        help=(
            "Author email address. "
            f"Default: value of git config user.email ({default_email})."
        ),
    )

    parser.add_argument(
        "--copyright-holder",
        default=default_name,
        help=(
            "Copyright holder name, such as an individual or company. "
            f"Default: {default_name}."
        ),
    )

    parser.add_argument(
        "--license",
        dest="license_id",
        default="LicenseRef-Proprietary",
        help=(
            "SPDX license identifier. Examples: MIT, Apache-2.0, "
            "BSD-3-Clause, or LicenseRef-Proprietary. "
            "Default: LicenseRef-Proprietary."
        ),
    )

    parser.add_argument(
        "--brief",
        default=None,
        help=(
            "Text for the Doxygen @brief field. "
            "When updating an existing header, the current value is "
            "preserved if this option is omitted."
        ),
    )

    parser.add_argument(
        "-r",
        "--recursive",
        action="store_true",
        help="Process supported files in subdirectories recursively.",
    )

    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Show the planned changes without modifying any files.",
    )

    args = parser.parse_args()

    files = collect_cpp_files(args.target, args.recursive)

    if not files:
        print(
            "No supported C or C++ source files were found.",
            file=sys.stderr,
        )
        return 1

    counts = {
        "added": 0,
        "updated": 0,
        "unchanged": 0,
        "skipped": 0,
        "error": 0,
    }

    for path in files:
        result = process_file(
            path=path,
            brief=args.brief,
            author_name=args.author_name,
            author_email=args.author_email,
            copyright_holder=args.copyright_holder,
            license_id=args.license_id,
            dry_run=args.dry_run,
        )

        counts[result] += 1

    print()
    print(f"Files scanned: {len(files)}")
    print(f"Headers added: {counts['added']}")
    print(f"Headers updated: {counts['updated']}")
    print(f"Files unchanged: {counts['unchanged']}")
    print(f"Files skipped: {counts['skipped']}")
    print(f"Errors: {counts['error']}")

    return 1 if counts["error"] else 0


if __name__ == "__main__":
    raise SystemExit(main())
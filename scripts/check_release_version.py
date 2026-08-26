#!/usr/bin/env python3
"""Validate ChainOSCPad release and Web Installer version references."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SEMVER_RE = re.compile(r"^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)$")
TARGETS = (
    ("ESP32-S3", "XIAO-ESP32S3"),
    ("ESP32-C6", "XIAO-ESP32C6"),
)


def read_text(relative_path: str) -> str:
    return (ROOT / relative_path).read_text(encoding="utf-8")


def require_match(pattern: str, text: str, description: str) -> str:
    match = re.search(pattern, text)
    if not match:
        raise SystemExit(f"Could not find {description}.")
    return match.group(1)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--tag", default="")
    parser.add_argument("--github-output", default="")
    parser.add_argument("--check-installer", action="store_true")
    parser.add_argument("--check-installer-metadata", action="store_true")
    args = parser.parse_args()

    version = require_match(
        r'APP_VERSION\[\]\s*=\s*"([^"]+)"',
        read_text("include/config.h"),
        "APP_VERSION in include/config.h",
    )
    if not SEMVER_RE.fullmatch(version):
        raise SystemExit(f"APP_VERSION is not valid SemVer: {version}")

    if args.tag:
        if not re.fullmatch(r"v(?:0|[1-9]\d*)\.(?:0|[1-9]\d*)\.(?:0|[1-9]\d*)", args.tag):
            raise SystemExit(f"Release tag must use vX.Y.Z format: {args.tag}")
        if args.tag[1:] != version:
            raise SystemExit(f"Tag version {args.tag[1:]} does not match APP_VERSION {version}.")

    manifest = None
    if args.check_installer or args.check_installer_metadata:
        manifest = json.loads(read_text("docs/installer/manifest.json"))
        if str(manifest.get("version", "")) != version:
            raise SystemExit(f"manifest.json version does not match APP_VERSION {version}.")

        builds = manifest.get("builds", [])
        for chip_family, slug in TARGETS:
            matches = [build for build in builds if build.get("chipFamily") == chip_family]
            if len(matches) != 1:
                raise SystemExit(f"manifest.json must contain exactly one {chip_family} build.")
            expected_path = f"firmware/ChainOSCPad-{version}-{slug}-merged.bin"
            parts = matches[0].get("parts", [])
            if len(parts) != 1 or parts[0].get("path") != expected_path or parts[0].get("offset") != 0:
                raise SystemExit(f"manifest.json must reference {expected_path} at offset 0.")
            if args.check_installer and not (ROOT / "docs" / "installer" / expected_path).is_file():
                raise SystemExit(f"Web Installer firmware file does not exist: docs/installer/{expected_path}")

        if f"Stable version {version}" not in read_text("docs/installer/index.html"):
            raise SystemExit("Installer index does not show the current stable version.")
        if f"現在の公開版は`{version}`です。" not in read_text("docs/installer/README.md"):
            raise SystemExit("Installer README does not show the current version.")

    print(f"Validated release version {version}")
    for _, slug in TARGETS:
        print(f"Firmware: ChainOSCPad-{version}-{slug}-merged.bin")
    if args.github_output:
        with Path(args.github_output).open("a", encoding="utf-8") as output:
            output.write(f"version={version}\n")


if __name__ == "__main__":
    main()

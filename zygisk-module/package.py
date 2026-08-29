#!/usr/bin/env python3
"""Build the Magisk module ZIP.

Two properties matter and neither survives a naive zip of a Windows checkout:

* Shell scripts must use LF.  With core.autocrlf=true the working tree holds
  CRLF, and Magisk's shell then reads every path with a trailing \\r, so the
  whole service script fails silently and its background loop never runs.
* Scripts must carry the executable bit.  Python's zipfile writes mode 0600
  by default, which does not survive as something Magisk will run.

Usage: python package.py [output.zip]
"""

import stat
import sys
import zipfile
from pathlib import Path

MODULE_DIR = Path(__file__).resolve().parent / "module"
DEFAULT_OUTPUT = Path(__file__).resolve().parent / "pikmin-nectar-rpc-v152.zip"

# Shipped to the device and parsed by a POSIX shell; must be LF.
TEXT_SUFFIXES = {".sh", ".prop"}
TEXT_NAMES = {"update-binary", "updater-script"}
# Magisk executes these directly.
EXECUTABLE_NAMES = {"service.sh", "post-fs-data.sh", "customize.sh", "update-binary"}


def is_text(path: Path) -> bool:
    return path.suffix in TEXT_SUFFIXES or path.name in TEXT_NAMES


def main() -> int:
    output = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_OUTPUT
    if not MODULE_DIR.is_dir():
        print(f"module directory not found: {MODULE_DIR}", file=sys.stderr)
        return 1

    files = sorted(p for p in MODULE_DIR.rglob("*") if p.is_file())
    if not files:
        print(f"no files under {MODULE_DIR}", file=sys.stderr)
        return 1

    with zipfile.ZipFile(output, "w", zipfile.ZIP_DEFLATED) as archive:
        for path in files:
            arcname = path.relative_to(MODULE_DIR).as_posix()
            data = path.read_bytes()
            if is_text(path):
                # Normalise CRLF (and lone CR) to LF.
                data = data.replace(b"\r\n", b"\n").replace(b"\r", b"\n")
            info = zipfile.ZipInfo(arcname)
            mode = 0o755 if path.name in EXECUTABLE_NAMES else 0o644
            info.external_attr = (stat.S_IFREG | mode) << 16
            info.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(info, data)
            print(f"  {arcname}  mode={oct(mode)}  {len(data)} bytes")

    print(f"\nwrote {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

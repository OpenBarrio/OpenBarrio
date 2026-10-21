"""Builds the release zip of the save mover with fixed timestamps and order, so the same build gives the same zip.

Usage (after `cmake --build --preset x86-release`): python tools/package.py
Writes dist/OpenBarrio-<version>.zip and prints its SHA-256.
"""
import hashlib
import os
import sys
import zipfile

VERSION = "0.1.0"
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
FILES = [  # (name in the zip, source)
    ("OpenBarrio.exe", "build86/bin/Release/OpenBarrio.exe"),
    ("openbarrio_redirect.dll", "build86/bin/Release/openbarrio_redirect.dll"),
    ("README.txt", "packaging/README.txt"),
    ("LICENSE", "LICENSE"),
]
FIXED_TIME = (2026, 1, 1, 0, 0, 0)


def main():
    out = os.path.join(ROOT, "dist", f"OpenBarrio-{VERSION}.zip")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    with zipfile.ZipFile(out, "w") as archive:
        for name, source in sorted(FILES):
            path = os.path.join(ROOT, source)
            if not os.path.isfile(path):
                sys.exit(f"missing {source}: build first")
            data = open(path, "rb").read()
            if name.endswith(".txt"):
                data = data.replace(b"\r\n", b"\n").replace(b"\n", b"\r\n")  # Notepad-friendly, whatever git did
            info = zipfile.ZipInfo(name, FIXED_TIME)
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = 0o644 << 16
            info.create_system = 0
            archive.writestr(info, data, compresslevel=9)
    digest = hashlib.sha256(open(out, "rb").read()).hexdigest()
    print(f"{os.path.relpath(out, ROOT)}  sha256 {digest}")


if __name__ == "__main__":
    main()

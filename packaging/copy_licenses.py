#!/usr/bin/env python3
"""Copy NOTICE + LICENSES (+ optional Qt SDK licenses) into a packaging directory."""
import os
import shutil
import sys

def main():
    if len(sys.argv) < 2:
        print("usage: copy_licenses.py DEST_DIR", file=sys.stderr)
        return 1
    dest = sys.argv[1]
    root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    os.makedirs(dest, exist_ok=True)
    license_dest = os.path.join(dest, "licenses")
    os.makedirs(license_dest, exist_ok=True)
    shutil.copy2(os.path.join(root, "NOTICE.txt"), os.path.join(dest, "NOTICE.txt"))
    for name in ("MIT.txt", "LGPL-3.0.txt", "GPL-3.0.txt"):
        shutil.copy2(os.path.join(root, "LICENSES", name), os.path.join(license_dest, name))
    qt_root = os.environ.get("QTDIR") or os.environ.get("Qt6_DIR") or os.environ.get("QT_ROOT_DIR")
    if qt_root:
        qt_lic = os.path.join(qt_root, "LICENSES")
        if os.path.isdir(qt_lic):
            qt_dest = os.path.join(license_dest, "qt")
            if os.path.isdir(qt_dest):
                shutil.rmtree(qt_dest)
            shutil.copytree(qt_lic, qt_dest)
    return 0

if __name__ == "__main__":
    raise SystemExit(main())

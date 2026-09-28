#!/bin/sh
set -eu
# Build a .deb from a linuxdeploy AppDir.
# Usage: build_deb.sh VERSION APPDIR OUT_DEB
VERSION="$1"
APPDIR="$2"
OUT_DEB="$3"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

mkdir -p \
  "$STAGE/DEBIAN" \
  "$STAGE/opt/GodOfPixels3" \
  "$STAGE/usr/share/applications" \
  "$STAGE/usr/share/mime/packages" \
  "$STAGE/usr/share/icons/hicolor/256x256/apps" \
  "$STAGE/usr/share/doc/godofpixels3"

cp -a "$APPDIR/usr/." "$STAGE/opt/GodOfPixels3/"
sed "s/^Version: .*/Version: $VERSION/" "$ROOT/packaging/linux/debian/control" > "$STAGE/DEBIAN/control"
cp "$ROOT/packaging/linux/debian/copyright" "$STAGE/usr/share/doc/godofpixels3/copyright"
cp "$ROOT/packaging/linux/debian/postinst" "$STAGE/DEBIAN/postinst"
cp "$ROOT/packaging/linux/debian/postrm" "$STAGE/DEBIAN/postrm"
chmod 0755 "$STAGE/DEBIAN/postinst" "$STAGE/DEBIAN/postrm"

cp "$ROOT/packaging/linux/godofpixels3.xml" "$STAGE/usr/share/mime/packages/godofpixels3.xml"
cp "$ROOT/res/images/logo.png" "$STAGE/usr/share/icons/hicolor/256x256/apps/godofpixels3.png"

cat > "$STAGE/usr/share/applications/godofpixels3.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=God of Pixels 3
Comment=Procedural planet editor
Exec=/opt/GodOfPixels3/bin/GodOfPixels3 %f
Icon=godofpixels3
Terminal=false
Categories=Graphics;Education;
MimeType=application/x-godofpixels3-planet;
StartupWMClass=GodOfPixels3
EOF

INSTALLED_SIZE=$(du -sk "$STAGE" | cut -f1)
echo "Installed-Size: $INSTALLED_SIZE" >> "$STAGE/DEBIAN/control"

dpkg-deb --build "$STAGE" "$OUT_DEB"

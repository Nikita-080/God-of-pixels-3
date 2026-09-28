# Agent notes

## Licensing (do not violate)

- Repository code is **MIT**. Do not relicense the app as GPL “because of Qt”.
- Bundled Qt is **LGPLv3 only**, and **must stay dynamically linked** (`windeployqt` / shared `.so` / Frameworks). Do not static-link Qt.
- Do not add GPL-only Qt modules to `QT +=`, including Charts, Graphs, Data Visualization, Quick 3D, Virtual Keyboard, MQTT, HTTP Server, Lottie, Network Authorization, `qtshadertools`.
- Keep `NOTICE.txt`, `LICENSES/`, `licenses.qrc`, and the Information → Licenses menu in the app and in every shipped package (zip, setup, `.app`, AppImage, `.deb`).
- Do not add a “no reverse engineering” clause to the installer. LGPL requires that users can replace Qt libraries.
- Do not embed proprietary fonts (including Consolas `.ttf`). Requesting the system font family by name is allowed.
- Before adding a third-party dependency: confirm it is compatible with MIT + LGPLv3, add it to `NOTICE.txt`, and put its license text in `LICENSES/`.

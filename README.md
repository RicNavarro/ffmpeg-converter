<p align="center">
  <img src="linux/icons/ffmpeg-converter-256.png" width="96" alt="FFmpeg Converter icon"/>
</p>

<h1 align="center">FFmpeg Converter</h1>

<p align="center">
  A lightweight Qt6 GUI wrapper for FFmpeg — convert video and audio files without touching the terminal.
</p>

<p align="center">
  <img alt="Platform" src="https://img.shields.io/badge/platform-Ubuntu%20%7C%20Debian-orange?logo=ubuntu"/>
  <img alt="Language" src="https://img.shields.io/badge/language-C%2B%2B17-blue?logo=cplusplus"/>
  <img alt="Qt" src="https://img.shields.io/badge/Qt-6.x-41CD52?logo=qt"/>
  <img alt="License" src="https://img.shields.io/badge/license-MIT-green"/>
</p>

---

## Overview

I work in a context where the video playback software we use is quite strict about file formats and encoder specifications. This program was built so I wouldn't have to open ffmpeg in the terminal every time a file needed to be converted — and so I wouldn't have to walk someone else through the details of ffmpeg commands either.

Technically, **FFmpeg Converter** is a native desktop application that wraps FFmpeg with a clean graphical interface. It supports drag & drop input, a wide range of output formats (MP4, MKV, WebM, MP3, FLAC, and more), advanced settings (codec, resolution, bitrate), a real-time progress bar, and a live log of the ffmpeg output. The resulting file can be sent straight to any folder you choose.

---

## Screenshot

> _Add a screenshot of the application window here._
> `![FFmpeg Converter UI](docs/screenshot.png)`

---

## Compatibility

This project was developed and tested on **Pop!OS 22.04** and **Ubuntu 22.04 LTS**. Any Debian-based distribution that provides Qt6 via `apt` should work without changes.

| Distribution | Status |
|---|---|
| Pop!OS 22.04 / 24.04 | ✅ Tested |
| Ubuntu 22.04 LTS | ✅ Tested |
| Ubuntu 24.04 LTS | ✅ Expected to work |
| Debian 12 (Bookworm) | ✅ Expected to work |
| Fedora / Arch / others | ⚠️ Not officially supported — may work with manual Qt6 setup |

> Non-Debian distributions are technically possible but require adjusting the build and packaging steps manually. The `.deb` packaging step will not apply.

---

## Prerequisites

| Package | Min. version | Required |
|---|---|---|
| `g++` | 11+ | Yes |
| `cmake` | 3.20+ | Yes |
| `qt6-base-dev` | 6.2+ | Yes |
| `libgl1-mesa-dev` | any | Yes |
| `libegl1-mesa-dev` | any | Yes |
| `ffmpeg` | 4.0+ | Yes |

---

## Installing dependencies

Run the following in your terminal:

```bash
sudo apt update
sudo apt install -y \
    cmake \
    g++ \
    ffmpeg \
    qt6-base-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libegl1-mesa-dev
```

Verify everything is in place:

```bash
cmake --version      # cmake version 3.x.x
g++ --version        # g++ (Ubuntu ...) 11.x.x
ffmpeg -version      # ffmpeg version x.x
pkg-config --modversion Qt6Core  # 6.x.x
```

---

## Icons

The application icons are already included in this repository at [`linux/icons/`](linux/icons/):

| File | Size | Used for |
|---|---|---|
| [`ffmpeg-converter-256.png`](linux/icons/ffmpeg-converter-256.png) | 256×256 | App launcher, `.desktop` entry |
| [`ffmpeg-converter-128.png`](linux/icons/ffmpeg-converter-128.png) | 128×128 | Taskbar, smaller contexts |

<p>
  <img src="linux/icons/ffmpeg-converter-128.png" width="64" alt="128px icon"/>
  &nbsp;&nbsp;
  <img src="linux/icons/ffmpeg-converter-256.png" width="64" alt="256px icon"/>
</p>

No action needed — the build script picks them up automatically from that path.

---

## Building

Clone the repository or download the source zip from the [Releases](#releases) tab.

```bash
git clone https://github.com/RicNavarro/ffmpeg-converter.git
cd ffmpeg-converter
chmod +x deploy-linux.sh
```

### Primary build command

```bash
QT_PREFIX=/usr ./deploy-linux.sh
```

This is the recommended approach for Ubuntu/Debian systems where Qt6 was installed via `apt`. The script will:

1. Verify all dependencies are present
2. Configure the project with CMake
3. Compile in Release mode (parallel jobs)
4. Generate a `.deb` package via CPack

### Build options

| Variable | Default | Description |
|---|---|---|
| `QT_PREFIX` | _(auto)_ | Path to Qt6 install prefix. Use `/usr` for system Qt6 |
| `BUILD_DIR` | `build-release` | Output directory for build artifacts |

```bash
# Custom build directory
BUILD_DIR=my-build QT_PREFIX=/usr ./deploy-linux.sh

# If Qt6 was installed manually (not via apt)
QT_PREFIX=~/Qt/6.7.0/gcc_64 ./deploy-linux.sh
```

### Alternative: manual CMake build

If you prefer to run CMake directly:

```bash
mkdir build-release && cd build-release
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=/usr
cmake --build . --parallel $(nproc)
```

---

## Installing the .deb

After a successful build, install the generated package:

```bash
sudo apt install ./build-release/ffmpeg-converter-1.0.0-Linux.deb
```

This will:

- Install the binary to `/usr/bin/FFmpegWrapper`
- Register the app in the **GNOME / Pop!_OS launcher** (search for "FFmpeg Converter" in Activities)
- Associate the app with common **video and audio MIME types** (right-click → Open With)
- Add an entry to the system package manager, enabling clean uninstallation

---

## Uninstalling

```bash
sudo apt remove ffmpeg-converter
```

This removes all installed files and launcher entries. It does **not** remove any files you converted.

---

## Troubleshooting

**`Could NOT find WrapOpenGL` during CMake configure**

The Qt6 GUI module requires OpenGL development headers. Install them and retry:

```bash
sudo apt install libgl1-mesa-dev libglu1-mesa-dev libegl1-mesa-dev
rm -rf build-release
QT_PREFIX=/usr ./deploy-linux.sh
```

---

**`Qt6 not found` even after installing `qt6-base-dev`**

The script's `pkg-config` check sometimes fails on certain Ubuntu/Debian configurations. Pass the prefix explicitly:

```bash
QT_PREFIX=/usr ./deploy-linux.sh
```

Or confirm Qt6 is present at `/usr`:

```bash
ls /usr/lib/x86_64-linux-gnu/cmake/Qt6/Qt6Config.cmake
```

---

**The app opens but conversion fails immediately**

ffmpeg is not being found at runtime. Confirm it is installed and on PATH:

```bash
which ffmpeg        # expected: /usr/bin/ffmpeg
ffmpeg -version     # should print version info
```

If ffmpeg is missing, install it:

```bash
sudo apt install ffmpeg
```

---

## Repository structure

```
ffmpeg-converter/
│
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── FormatProfile.h / .cpp        — conversion job data struct
│   │   ├── FFmpegPresets.h / .cpp        — format → codec → ffmpeg args mapping
│   │   ├── ConversionWorker.h / .cpp     — QThread + QProcess + progress parser
│   │   ├── ConversionController.h / .cpp — job queue orchestrator
│   │   └── AppSettings.h / .cpp          — persistent user preferences (QSettings)
│   ├── ui/
│   │   ├── MainWindow.h / .cpp           — main window and layout
│   │   ├── DropZone.h / .cpp             — drag & drop file input widget
│   │   ├── SettingsPanel.h / .cpp        — format, codec, resolution, bitrate
│   │   └── ConversionLog.h / .cpp        — live log panel with Copy / Clear
│   └── utils/
│       └── FileUtils.h / .cpp            — conflict resolution, open folder, human sizes
│
├── linux/
│   ├── ffmpeg-converter.desktop          — freedesktop app entry (launcher + MIME)
│   ├── deb/
│   │   ├── postinst                      — post-install hook (icon cache, desktop db)
│   │   └── postrm                        — post-remove hook
│   └── icons/
│       ├── ffmpeg-converter-256.png      — 256×256 application icon
│       └── ffmpeg-converter-128.png      — 128×128 application icon
│
├── resources/
│   ├── resources.qrc                     — Qt resource file
│   └── app.rc                            — Windows resource file (icon + version metadata)
│
├── CMakeLists.txt                        — build system (cross-platform)
├── deploy-linux.sh                       — Linux build + .deb packaging script
├── deploy.ps1                            — Windows build + NSIS installer script
└── installer/
    ├── installer.nsi                     — NSIS installer script (Windows)
    └── generate-welcome-bmp.ps1          — generates the wizard sidebar image
```

> The full source tree is browsable directly on GitHub. Compiled releases (`.deb` for Linux, `Setup.exe` for Windows) are available under the [**Releases**](../../releases) tab as `.zip` archives.

---

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-change`
3. Commit your changes: `git commit -m "add: description of change"`
4. Push to the branch: `git push origin feature/my-change`
5. Open a Pull Request

---

## License

This project is licensed under the [MIT License](LICENSE).

## ゲームを開始する手順

### 1. 依存関係のインストール

#### macOS
```bash
# Xcode Command Line Tools (必須)
xcode-select --install

# Homebrewでライブラリをインストール
brew install cmake glfw glm
```

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake libglfw3-dev libglm-dev libgl1-mesa-dev
```

#### Windows
```cmd
# Chocolateyを使用する場合
choco install cmake glfw glm

# または、手動でインストール
# 1. CMake: https://cmake.org/download/ からダウンロード
# 2. GLFW: https://www.glfw.org/download.html からダウンロード
# 3. GLM: https://github.com/g-truc/glm/releases からダウンロード

# ビルドツール（必須）
# Visual Studio Build Tools または MinGW-w64 をインストール
# https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
```

### 2. ゲームの起動

#### macOS/Linux
```bash
./start_game.sh
```

#### Windows
```cmd
start_game.bat
```

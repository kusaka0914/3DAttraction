@echo off
echo 🎮 Vulkan3D ゲームを開始します...

REM ビルドディレクトリが存在しない場合は作成
if not exist "build" (
    echo 📁 ビルドディレクトリを作成中...
    mkdir build
)

cd build

REM ビルドが必要かチェック
if not exist "Vulkan3D.exe" (
    echo 🔨 ゲームをビルド中...
    cmake .. && cmake --build .
    if errorlevel 1 (
        echo ❌ ビルドに失敗しました。依存関係を確認してください。
        pause
        exit /b 1
    )
    echo ✅ ビルド完了！
) else (
    echo ✅ 既存のビルドを使用します
)

echo 🚀 ゲームを起動中...
Vulkan3D.exe
pause

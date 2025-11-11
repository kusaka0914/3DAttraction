@echo off
echo ゲームを開始します。

REM ビルドディレクトリが存在しない場合は作成
if not exist "build" (
    echo ビルドディレクトリを作成しています・・・
    mkdir build
)

cd build

REM ビルドが必要かチェック
if not exist "SlimesSkyTravel.exe" (
    echo ゲームをビルドしています・・・
    cmake .. && cmake --build .
    if errorlevel 1 (
        echo ビルドに失敗しました。依存関係を確認してください。
        pause
        exit /b 1
    )
    echo ビルドが完了しました。
) else (
    echo 既存のビルドを使用します。
)

echo ゲームを起動しています・・・
SlimesSkyTravel.exe
pause
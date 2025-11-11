#!/bin/bash

echo "ゲームを開始します。"

# ビルドディレクトリが存在しない場合は作成
if [ ! -d "build" ]; then
    echo "ビルドディレクトリを作成しています・・・"
    mkdir -p build
fi

cd build

# 実行ファイル名を決定（macOSでは拡張子なし、Windowsでは.exe）
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    EXECUTABLE="SlimesSkyTravel.exe"
else
    EXECUTABLE="SlimesSkyTravel"
fi

# ビルドが必要かチェック
if [ ! -f "$EXECUTABLE" ] || [ "../CMakeLists.txt" -nt "$EXECUTABLE" ]; then
    echo "ゲームをビルドしています・・・"
    cmake .. && make
    if [ $? -ne 0 ]; then
        echo "ビルドに失敗しました。依存関係を確認してください。"
        exit 1
    fi
    echo "ビルドが完了しました。"
else
    echo "既存のビルドを使用します。"
fi

echo "ゲームを起動しています・・・"
./"$EXECUTABLE"
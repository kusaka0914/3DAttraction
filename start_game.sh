#!/bin/bash

echo "🎮 Vulkan3D ゲームを開始します..."

# ビルドディレクトリが存在しない場合は作成
if [ ! -d "build" ]; then
    echo "📁 ビルドディレクトリを作成中..."
    mkdir -p build
fi

cd build

# ビルドが必要かチェック
if [ ! -f "Vulkan3D" ] || [ "../CMakeLists.txt" -nt "Vulkan3D" ]; then
    echo "🔨 ゲームをビルド中..."
    cmake .. && make
    if [ $? -ne 0 ]; then
        echo "❌ ビルドに失敗しました。依存関係を確認してください。"
        exit 1
    fi
    echo "✅ ビルド完了！"
else
    echo "✅ 既存のビルドを使用します"
fi

echo "🚀 ゲームを起動中..."
./Vulkan3D


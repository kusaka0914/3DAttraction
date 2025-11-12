#pragma once

#include <string>
#include <fstream>
#include <vector>

namespace ResourcePath {
    // リソースファイルのパスを取得（複数の候補を試す）
    // 開発中（buildディレクトリから実行）でもリリース（実行ファイルと同じディレクトリにassetsがある）でも動作
    inline std::string getResourcePath(const std::string& relativePath) {
        // 試すパスのリスト
        std::vector<std::string> candidates = {
            relativePath,                    // ./assets/... (実行ファイルの隣、リリースビルド用)
            "../" + relativePath,            // ../assets/... (buildディレクトリから実行)
            "../../" + relativePath,         // ../../assets/... (build/Releaseディレクトリから実行)
        };
        
        // 各候補を試してファイルが存在するか確認
        for (const auto& path : candidates) {
            std::ifstream file(path);
            if (file.good()) {
                file.close();
                return path;
            }
        }
        
        // 見つからない場合は元のパスを返す（エラー処理は呼び出し側で）
        return relativePath;
    }
}


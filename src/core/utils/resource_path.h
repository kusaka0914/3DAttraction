#pragma once

#include <string>
#include <fstream>
#include <vector>

/**
 * @brief リソースパスユーティリティ
 * @details リソースファイルのパス解決を提供します。
 */
namespace ResourcePath {
    /**
     * @brief リソースファイルのパスを取得する
     * @details 複数の候補パスを試してリソースファイルを探します。
     * 開発環境（buildディレクトリから実行）とリリース環境（実行ファイルと同じディレクトリにassetsがある）の両方に対応します。
     * 
     * @param relativePath 相対パス
     * @return 実際のリソースパス
     */
    inline std::string getResourcePath(const std::string& relativePath) {
        std::vector<std::string> candidates = {
            relativePath,
            "../" + relativePath,
            "../../" + relativePath,
        };
        
        for (const auto& path : candidates) {
            std::ifstream file(path);
            if (file.good()) {
                file.close();
                return path;
            }
        }
        
        return relativePath;
    }
}

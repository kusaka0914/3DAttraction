#ifndef DEBUG_CONFIG_H
#define DEBUG_CONFIG_H

// デバッグ出力の制御フラグ
// 0 = デバッグ無効（リリース用）
// 1 = デバッグ有効（開発用）
#define ENABLE_DEBUG_OUTPUT 1

// デバッグ出力マクロ
#if ENABLE_DEBUG_OUTPUT
    #define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
    #define DEBUG_PRINTF(...)  // 何もしない（完全に無効化）
#endif

#endif // DEBUG_CONFIG_H

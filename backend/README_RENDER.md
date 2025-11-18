# Render デプロイ手順

## 1. Renderアカウント作成
1. https://render.com にアクセス
2. GitHubアカウントでサインアップ

## 2. 新しいWebサービスを作成
1. Renderダッシュボードで「New +」→「Web Service」を選択
2. GitHubリポジトリを接続
3. 以下の設定を行う：
   - **Name**: `slimes-sky-travel-api`
   - **Environment**: `Node`
   - **Build Command**: `npm install`
   - **Start Command**: `node server.js`
   - **Plan**: `Free`（無料プラン）

## 3. 環境変数の設定（オプション）
Renderダッシュボードで環境変数を設定できます：
- `NODE_ENV`: `production`
- `PORT`: `3000`（Renderが自動的に設定するため不要な場合もあり）

## 4. デプロイ
1. 「Create Web Service」をクリック
2. デプロイが完了するまで待つ（数分かかります）

## 5. URLの取得
デプロイ完了後、Renderが以下のようなURLを提供します：
- `https://slimes-sky-travel-api.onrender.com`

## 6. ゲーム側の設定
`assets/config/leaderboard_config.json`の`baseUrl`をRenderのURLに変更：

```json
{
  "baseUrl": "https://slimes-sky-travel-api.onrender.com",
  "enabled": true,
  "timeoutSeconds": 5
}
```

## 注意事項
- **無料プラン**: 15分間アクセスがないとスリープします。次回アクセス時に自動起動しますが、初回は5-30秒かかります。
- **常時起動が必要な場合**: `Starter`プラン（$7/月）にアップグレードしてください。


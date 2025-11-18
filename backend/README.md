# Slime's Sky Travel Leaderboard API

タイムアタック記録をオンラインで共有するためのバックエンドAPIサーバーです。

## セットアップ

### 1. Node.jsのインストール

Node.js v16以上が必要です。インストールされていない場合は以下からダウンロードしてください：
- https://nodejs.org/

### 2. 依存関係のインストール

```bash
cd backend
npm install
```

### 3. サーバーの起動

```bash
# 通常起動
npm start

# 開発モード（ファイル変更時に自動リロード）
npm run dev
```

サーバーは `http://localhost:3000` で起動します。

## API エンドポイント

### GET /api/health
ヘルスチェックエンドポイント

**レスポンス例:**
```json
{
  "status": "ok",
  "message": "Leaderboard API is running"
}
```

### GET /api/leaderboard/:stageNumber
ステージ別ランキング取得（トップ10）

**パラメータ:**
- `stageNumber`: ステージ番号（1-5）

**例:**
```bash
curl http://localhost:3000/api/leaderboard/1
```

**レスポンス例:**
```json
{
  "stageNumber": 1,
  "records": [
    {
      "playerName": "Player1",
      "time": 12.34,
      "timestamp": "2024-01-01 00:00:00"
    },
    {
      "playerName": "Player2",
      "time": 13.45,
      "timestamp": "2024-01-01 01:00:00"
    }
  ]
}
```

### POST /api/leaderboard
タイム記録の送信

**リクエストボディ:**
```json
{
  "stageNumber": 1,
  "time": 12.34,
  "playerName": "Player1"
}
```

**例:**
```bash
curl -X POST http://localhost:3000/api/leaderboard \
  -H "Content-Type: application/json" \
  -d '{"stageNumber": 1, "time": 12.34, "playerName": "TestPlayer"}'
```

**レスポンス例:**
```json
{
  "success": true,
  "id": 1,
  "message": "Record saved successfully"
}
```

### GET /api/leaderboard/global/top
全ステージのトップ記録取得

**例:**
```bash
curl http://localhost:3000/api/leaderboard/global/top
```

**レスポンス例:**
```json
{
  "topRecords": {
    "1": {
      "playerName": "Player1",
      "time": 12.34,
      "timestamp": "2024-01-01 00:00:00"
    },
    "2": {
      "playerName": "Player2",
      "time": 15.67,
      "timestamp": "2024-01-01 01:00:00"
    }
  }
}
```

## データベース

SQLiteを使用しています。データベースファイル（`leaderboard.db`）は自動的に作成されます。

### テーブル構造

```sql
CREATE TABLE leaderboard (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    stageNumber INTEGER NOT NULL,
    playerName TEXT NOT NULL,
    time REAL NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

## デプロイ

### Heroku

1. Heroku CLIをインストール
2. プロジェクトルートで以下を実行:
```bash
heroku create your-app-name
git push heroku main
```

### Railway

1. Railwayアカウントを作成
2. GitHubリポジトリを接続
3. `backend`ディレクトリをルートとして設定

### Render

1. Renderアカウントを作成
2. New Web Serviceからリポジトリを選択
3. Root Directory: `backend`
4. Build Command: `npm install`
5. Start Command: `npm start`

## 環境変数

- `PORT`: サーバーのポート番号（デフォルト: 3000）

## 注意事項

- 本番環境ではCORS設定を適切に制限してください
- レート制限の実装を検討してください
- 認証機能の追加を検討してください


# PostgreSQL移行手順

## 問題
Renderの無料プランでは、ephemeral storage（一時ストレージ）が使われるため、SQLiteファイルはサービス再起動時に失われます。

## 解決策
PostgreSQLに移行することで、データを永続化できます。Renderの無料プランでもPostgreSQLは利用可能です。

## 移行手順

### 1. RenderでPostgreSQLデータベースを作成
1. Renderダッシュボードで「New +」→「PostgreSQL」を選択
2. 以下の設定を行う：
   - **Name**: `slimes-sky-travel-db`
   - **Database**: `leaderboard`（自動生成される）
   - **User**: `slimes_user`（自動生成される）
   - **Plan**: `Free`（無料プラン）
3. 「Create Database」をクリック
4. データベース作成後、**Internal Database URL**をコピー（例：`postgresql://slimes_user:password@dpg-xxxxx-a.oregon-postgres.render.com/leaderboard`）

### 2. WebサービスにPostgreSQLを接続
1. Webサービスの設定画面を開く
2. 「Environment」セクションで、`DATABASE_URL`環境変数を追加
3. 値に、PostgreSQLの**Internal Database URL**を貼り付け
4. 「Save Changes」をクリック

### 3. コードを更新
1. `package.json`の依存関係を更新（既に`pg`に変更済み）
2. `server.js`を`server_postgres.js`の内容で置き換える
3. または、`server.js`を直接編集してPostgreSQL対応にする

### 4. デプロイ
1. 変更をコミット・プッシュ
2. Renderが自動的に再デプロイ
3. デプロイ完了後、ヘルスチェックで確認

## 注意事項
- PostgreSQLの無料プランでは、90日間アクセスがないとデータベースが削除される可能性があります
- 本番環境では、定期的にバックアップを取得することを推奨します
- データベースの接続情報は環境変数で管理し、コードに直接書かないでください

## ローカル開発時の設定
ローカルで開発する場合、`.env`ファイルを作成：

```env
DATABASE_URL=postgresql://localhost:5432/leaderboard
```

または、PostgreSQLをインストールしてローカルで実行：

```bash
# macOS
brew install postgresql
brew services start postgresql
createdb leaderboard

# Linux
sudo apt-get install postgresql postgresql-contrib
sudo systemctl start postgresql
sudo -u postgres createdb leaderboard
```


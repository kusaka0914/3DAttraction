# PostgreSQL移行ガイド

## ✅ コードの移行は完了しました

`server.js`をPostgreSQL対応版に更新しました。次に、RenderでPostgreSQLデータベースを設定してください。

## 📋 Renderでの設定手順

### ステップ1: PostgreSQLデータベースを作成

1. Renderダッシュボード（https://dashboard.render.com）にログイン
2. 「New +」ボタンをクリック
3. 「PostgreSQL」を選択
4. 以下の設定を入力：
   - **Name**: `slimes-sky-travel-db`（任意の名前）
   - **Database**: `leaderboard`（自動生成される）
   - **User**: `slimes_user`（自動生成される）
   - **Region**: 最寄りのリージョンを選択（例：Oregon）
   - **PostgreSQL Version**: 最新版を選択
   - **Plan**: `Free`（無料プラン）
5. 「Create Database」をクリック
6. データベース作成完了まで待つ（1-2分）

### ステップ2: Internal Database URLをコピー

1. 作成したPostgreSQLデータベースのページを開く
2. 「Connections」セクションを確認
3. **Internal Database URL**をコピー
   - 例：`postgresql://slimes_user:password@dpg-xxxxx-a.oregon-postgres.render.com/leaderboard`
   - ⚠️ **Internal Database URL**を使用してください（External Database URLではありません）

### ステップ3: Webサービスに環境変数を設定

1. Renderダッシュボードで、既存のWebサービス（`slimes-sky-travel-api`）を開く
2. 左側のメニューから「Environment」を選択
3. 「Add Environment Variable」をクリック
4. 以下の環境変数を追加：
   - **Key**: `DATABASE_URL`
   - **Value**: ステップ2でコピーした**Internal Database URL**を貼り付け
5. 「Save Changes」をクリック

### ステップ4: 再デプロイ

1. 「Manual Deploy」セクションで「Deploy latest commit」をクリック
2. または、GitHubに変更をプッシュすると自動的に再デプロイされます

### ステップ5: 動作確認

デプロイ完了後、以下のコマンドで確認：

```bash
# ヘルスチェック
curl https://slimes-sky-travel-api.onrender.com/api/health

# ランキング取得（空の配列が返るはず）
curl https://slimes-sky-travel-api.onrender.com/api/leaderboard/1
```

## 🔍 トラブルシューティング

### エラー: "relation does not exist"
- データベースの初期化が完了していない可能性があります
- 数秒待ってから再度アクセスしてください
- ログを確認して、テーブル作成が成功しているか確認してください

### エラー: "connection refused"
- `DATABASE_URL`が正しく設定されているか確認してください
- **Internal Database URL**を使用しているか確認してください（External URLではありません）

### データが保存されない
- トランザクションが正しくコミットされているかログで確認してください
- データベースの接続が確立されているか確認してください

## 📝 注意事項

- PostgreSQLの無料プランでは、90日間アクセスがないとデータベースが削除される可能性があります
- 本番環境では、定期的にバックアップを取得することを推奨します
- データベースの接続情報は環境変数で管理し、コードに直接書かないでください

## 🎉 完了

これで、データが永続化され、サービス再起動後も記録が残るようになります！


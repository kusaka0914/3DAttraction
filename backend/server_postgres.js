const express = require('express');
const cors = require('cors');
const bodyParser = require('body-parser');
const { Pool } = require('pg');
const path = require('path');

const app = express();
const PORT = process.env.PORT || 3000;

// ミドルウェア
app.use(cors()); // CORS許可（開発用）
app.use(bodyParser.json());

// PostgreSQL接続設定
// RenderのPostgreSQLを使用する場合、環境変数DATABASE_URLが自動的に設定される
// ローカル開発時は、.envファイルまたは環境変数で設定
const pool = new Pool({
    connectionString: process.env.DATABASE_URL || 'postgresql://localhost:5432/leaderboard',
    ssl: process.env.DATABASE_URL ? { rejectUnauthorized: false } : false
});

// データベース接続テスト
pool.on('connect', () => {
    console.log('Connected to PostgreSQL database');
});

pool.on('error', (err) => {
    console.error('Unexpected error on idle client', err);
    process.exit(-1);
});

// テーブル作成（初回起動時のみ）
async function initializeDatabase() {
    try {
        // leaderboardテーブル
        await pool.query(`
            CREATE TABLE IF NOT EXISTS leaderboard (
                id SERIAL PRIMARY KEY,
                stageNumber INTEGER NOT NULL,
                playerName TEXT NOT NULL,
                time REAL NOT NULL,
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        `);
        console.log('Leaderboard table created/verified');

        // インデックス作成
        await pool.query(`
            CREATE INDEX IF NOT EXISTS idx_stage_time 
            ON leaderboard(stageNumber, time)
        `);
        console.log('Index created/verified');

        // replaysテーブル
        await pool.query(`
            CREATE TABLE IF NOT EXISTS replays (
                id SERIAL PRIMARY KEY,
                leaderboardId INTEGER NOT NULL,
                stageNumber INTEGER NOT NULL,
                replayData TEXT NOT NULL,
                FOREIGN KEY (leaderboardId) REFERENCES leaderboard(id) ON DELETE CASCADE
            )
        `);
        console.log('Replays table created/verified');

        // replaysテーブルのインデックス
        await pool.query(`
            CREATE INDEX IF NOT EXISTS idx_replay_leaderboard 
            ON replays(leaderboardId)
        `);
        console.log('Replay index created/verified');
    } catch (err) {
        console.error('Database initialization error:', err);
        process.exit(1);
    }
}

// データベース初期化
initializeDatabase();

// ヘルスチェック
app.get('/api/health', (req, res) => {
    res.json({ status: 'ok', message: 'Leaderboard API is running' });
});

// ステージ別ランキング取得（トップ10）
app.get('/api/leaderboard/:stageNumber', async (req, res) => {
    const stageNumber = parseInt(req.params.stageNumber);
    
    if (isNaN(stageNumber) || stageNumber < 1 || stageNumber > 5) {
        return res.status(400).json({ error: 'Invalid stage number' });
    }
    
    try {
        const result = await pool.query(
            `SELECT l.id, l.playerName, l.time, l.timestamp, 
                    CASE WHEN r.id IS NOT NULL THEN true ELSE false END as "hasReplay"
             FROM leaderboard l
             LEFT JOIN replays r ON l.id = r."leaderboardId"
             WHERE l."stageNumber" = $1 
             ORDER BY l.time ASC 
             LIMIT 10`,
            [stageNumber]
        );
        
        const records = result.rows.map(row => ({
            id: row.id,
            playerName: row.playerName,
            time: parseFloat(row.time),
            timestamp: row.timestamp,
            hasReplay: row.hasReplay
        }));
        
        res.json({
            stageNumber: stageNumber,
            records: records
        });
    } catch (err) {
        console.error('Database error:', err);
        res.status(500).json({ error: 'Database error' });
    }
});

// タイム記録の送信
app.post('/api/leaderboard', async (req, res) => {
    const { stageNumber, time, playerName, replayData } = req.body;
    
    // バリデーション
    if (!stageNumber || !time || !playerName) {
        return res.status(400).json({ 
            error: 'Missing required fields: stageNumber, time, playerName' 
        });
    }
    
    const stageNum = parseInt(stageNumber);
    const timeValue = parseFloat(time);
    
    if (isNaN(stageNum) || stageNum < 1 || stageNum > 5) {
        return res.status(400).json({ error: 'Invalid stage number' });
    }
    
    if (isNaN(timeValue) || timeValue <= 0 || timeValue > 9999) {
        return res.status(400).json({ error: 'Invalid time value' });
    }
    
    if (playerName.length > 50 || playerName.length === 0) {
        return res.status(400).json({ error: 'Invalid player name' });
    }
    
    const client = await pool.connect();
    try {
        await client.query('BEGIN');
        
        // ランキングに挿入
        const leaderboardResult = await client.query(
            `INSERT INTO leaderboard ("stageNumber", "playerName", time) 
             VALUES ($1, $2, $3) 
             RETURNING id`,
            [stageNum, playerName.substring(0, 50), timeValue]
        );
        
        const leaderboardId = leaderboardResult.rows[0].id;
        
        // リプレイデータがあれば保存
        if (replayData) {
            const replayDataJson = JSON.stringify(replayData);
            await client.query(
                `INSERT INTO replays ("leaderboardId", "stageNumber", "replayData") 
                 VALUES ($1, $2, $3)`,
                [leaderboardId, stageNum, replayDataJson]
            );
            console.log(`Replay saved for leaderboard entry ${leaderboardId}`);
        }
        
        await client.query('COMMIT');
        
        res.json({
            success: true,
            id: leaderboardId,
            message: 'Record saved successfully'
        });
    } catch (err) {
        await client.query('ROLLBACK');
        console.error('Database error:', err);
        res.status(500).json({ error: 'Failed to save record' });
    } finally {
        client.release();
    }
});

// リプレイデータ取得
app.get('/api/replay/:leaderboardId', async (req, res) => {
    const leaderboardId = parseInt(req.params.leaderboardId);
    
    if (isNaN(leaderboardId) || leaderboardId <= 0) {
        return res.status(400).json({ error: 'Invalid leaderboard ID' });
    }
    
    try {
        const result = await pool.query(
            `SELECT "replayData" 
             FROM replays 
             WHERE "leaderboardId" = $1`,
            [leaderboardId]
        );
        
        if (result.rows.length === 0) {
            return res.status(404).json({ error: 'Replay not found' });
        }
        
        const replayData = JSON.parse(result.rows[0].replayData);
        res.json({
            success: true,
            replayData: replayData
        });
    } catch (err) {
        console.error('Database error:', err);
        if (err instanceof SyntaxError) {
            return res.status(500).json({ error: 'Failed to parse replay data' });
        }
        res.status(500).json({ error: 'Database error' });
    }
});

// 全ステージのトップ記録取得
app.get('/api/leaderboard/global/top', async (req, res) => {
    try {
        const result = await pool.query(
            `SELECT "stageNumber", "playerName", time, timestamp
             FROM leaderboard l1
             WHERE time = (
                 SELECT MIN(time)
                 FROM leaderboard l2
                 WHERE l2."stageNumber" = l1."stageNumber"
             )
             ORDER BY "stageNumber"`
        );
        
        const topRecords = {};
        result.rows.forEach(row => {
            topRecords[row.stageNumber] = {
                playerName: row.playerName,
                time: parseFloat(row.time),
                timestamp: row.timestamp
            };
        });
        
        res.json({ topRecords });
    } catch (err) {
        console.error('Database error:', err);
        res.status(500).json({ error: 'Database error' });
    }
});

// エラーハンドリング
app.use((err, req, res, next) => {
    console.error('Error:', err);
    res.status(500).json({ error: 'Internal server error' });
});

// サーバー起動
app.listen(PORT, () => {
    console.log(`Leaderboard API server running on http://localhost:${PORT}`);
    console.log(`Health check: http://localhost:${PORT}/api/health`);
});


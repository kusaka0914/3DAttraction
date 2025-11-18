const express = require('express');
const cors = require('cors');
const bodyParser = require('body-parser');
const sqlite3 = require('sqlite3').verbose();
const path = require('path');

const app = express();
const PORT = process.env.PORT || 3000;

// ミドルウェア
app.use(cors()); // CORS許可（開発用）
app.use(bodyParser.json());

// データベース初期化（同期的にテーブル作成を完了させる）
const dbPath = path.join(__dirname, 'leaderboard.db');
const db = new sqlite3.Database(dbPath, (err) => {
    if (err) {
        console.error('Database connection error:', err.message);
        process.exit(1);
    } else {
        console.log('Connected to SQLite database');
        
        // テーブル作成（同期的に実行）
        db.serialize(() => {
            db.run(`
                CREATE TABLE IF NOT EXISTS leaderboard (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    stageNumber INTEGER NOT NULL,
                    playerName TEXT NOT NULL,
                    time REAL NOT NULL,
                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
                )
            `, (err) => {
                if (err) {
                    console.error('Failed to create table:', err.message);
                    process.exit(1);
                } else {
                    console.log('Table created successfully');
                }
            });
            
            // インデックス作成（パフォーマンス向上）
            db.run(`
                CREATE INDEX IF NOT EXISTS idx_stage_time 
                ON leaderboard(stageNumber, time)
            `, (err) => {
                if (err) {
                    console.error('Failed to create index:', err.message);
                } else {
                    console.log('Index created successfully');
                }
            });
        });
    }
});

// ヘルスチェック
app.get('/api/health', (req, res) => {
    res.json({ status: 'ok', message: 'Leaderboard API is running' });
});

// ステージ別ランキング取得（トップ10）
app.get('/api/leaderboard/:stageNumber', (req, res) => {
    const stageNumber = parseInt(req.params.stageNumber);
    
    if (isNaN(stageNumber) || stageNumber < 1 || stageNumber > 5) {
        return res.status(400).json({ error: 'Invalid stage number' });
    }
    
    db.all(
        `SELECT playerName, time, timestamp 
         FROM leaderboard 
         WHERE stageNumber = ? 
         ORDER BY time ASC 
         LIMIT 10`,
        [stageNumber],
        (err, rows) => {
            if (err) {
                console.error('Database error:', err);
                return res.status(500).json({ error: 'Database error' });
            }
            
            const records = rows.map(row => ({
                playerName: row.playerName,
                time: row.time,
                timestamp: row.timestamp
            }));
            
            res.json({
                stageNumber: stageNumber,
                records: records
            });
        }
    );
});

// タイム記録の送信
app.post('/api/leaderboard', (req, res) => {
    const { stageNumber, time, playerName } = req.body;
    
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
    
    // データベースに挿入
    db.run(
        `INSERT INTO leaderboard (stageNumber, playerName, time) 
         VALUES (?, ?, ?)`,
        [stageNum, playerName.substring(0, 50), timeValue],
        function(err) {
            if (err) {
                console.error('Database error:', err);
                return res.status(500).json({ error: 'Failed to save record' });
            }
            
            res.json({
                success: true,
                id: this.lastID,
                message: 'Record saved successfully'
            });
        }
    );
});

// 全ステージのトップ記録取得
app.get('/api/leaderboard/global/top', (req, res) => {
    db.all(
        `SELECT stageNumber, playerName, time, timestamp
         FROM leaderboard l1
         WHERE time = (
             SELECT MIN(time)
             FROM leaderboard l2
             WHERE l2.stageNumber = l1.stageNumber
         )
         ORDER BY stageNumber`,
        [],
        (err, rows) => {
            if (err) {
                console.error('Database error:', err);
                return res.status(500).json({ error: 'Database error' });
            }
            
            const topRecords = {};
            rows.forEach(row => {
                topRecords[row.stageNumber] = {
                    playerName: row.playerName,
                    time: row.time,
                    timestamp: row.timestamp
                };
            });
            
            res.json({ topRecords });
        }
    );
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


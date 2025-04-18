/*
 * @Author: Nana5aki
 * @Date: 2025-04-18 21:33:56
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-04-18 22:14:57
 * @FilePath: /MySylar/scripts/download_http_parser.js
 */
const https = require('https');
const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');

// 配置
const HTTP_PARSER_URL = 'https://raw.githubusercontent.com/nodejs/http-parser/v2.9.4/http_parser.c';
const HTTP_PARSER_H_URL = 'https://raw.githubusercontent.com/nodejs/http-parser/v2.9.4/http_parser.h';
const THIRD_PARTY_DIR = path.join(__dirname, '..', 'third_party');
const HTTP_PARSER_DIR = path.join(THIRD_PARTY_DIR, 'http-parser');

// 确保目录存在
function ensureDir(dir) {
    if (!fs.existsSync(dir)) {
        fs.mkdirSync(dir, { recursive: true });
    }
}

// 下载文件
function downloadFile(url, dest) {
    return new Promise((resolve, reject) => {
        console.log(`正在下载 ${url}...`);
        const file = fs.createWriteStream(dest);
        let total = 0;
        let received = 0;

        https.get(url, (response) => {
            if (response.statusCode !== 200) {
                reject(new Error(`下载失败: ${response.statusCode} ${response.statusMessage}`));
                return;
            }

            total = parseInt(response.headers['content-length'], 10);
            response.pipe(file);

            response.on('data', (chunk) => {
                received += chunk.length;
                if (total) {
                    const percent = ((received / total) * 100).toFixed(2);
                    process.stdout.write(`\r下载进度: ${percent}%`);
                }
            });

            file.on('finish', () => {
                file.close();
                console.log('\n下载完成');
                resolve();
            });
        }).on('error', (err) => {
            fs.unlink(dest, () => {});
            reject(err);
        });

        file.on('error', (err) => {
            fs.unlink(dest, () => {});
            reject(err);
        });
    });
}

// 主函数
async function main() {
    try {
        // 确保目录存在
        ensureDir(THIRD_PARTY_DIR);
        ensureDir(HTTP_PARSER_DIR);

        // 下载文件
        await downloadFile(HTTP_PARSER_URL, path.join(HTTP_PARSER_DIR, 'http_parser.c'));
        await downloadFile(HTTP_PARSER_H_URL, path.join(HTTP_PARSER_DIR, 'http_parser.h'));

        console.log('http-parser 下载完成');
    } catch (err) {
        console.error('错误:', err.message);
        process.exit(1);
    }
}

main(); 
"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const path_1 = __importDefault(require("path"));
const fs_1 = __importDefault(require("fs"));
const dotenv_1 = __importDefault(require("dotenv"));
const express_1 = __importDefault(require("express"));
const app = (0, express_1.default)();
const port = 3001;
dotenv_1.default.config();
console.log("Env loaded:", {
    SAMPLE_DIR: process.env.SAMPLE_DIR,
    WEBAPP_DIR: process.env.WEBAPP_DIR,
});
const SAMPLE_DIR = process.env.SAMPLE_DIR;
const WEBAPP_DIR = process.env.WEBAPP_DIR;
if (!SAMPLE_DIR) {
    console.error("Please set the SAMPLE_DIR environment variable");
    process.exit(1);
}
if (!WEBAPP_DIR) {
    console.error("Please set the WEBAPP_DIR environment variable");
    process.exit(1);
}
// Recursive function to list all files relative to SAMPLE_DIR
function listFilesRelative(dir, baseDir = dir) {
    let results = [];
    const entries = fs_1.default.readdirSync(dir, { withFileTypes: true });
    for (const entry of entries) {
        const fullPath = path_1.default.join(dir, entry.name);
        if (entry.isDirectory()) {
            results = results.concat(listFilesRelative(fullPath, baseDir));
        }
        else {
            results.push(path_1.default.relative(baseDir, fullPath));
        }
    }
    return results;
}
// /files endpoint
app.get('/files', (req, res) => {
    try {
        const files = listFilesRelative(SAMPLE_DIR);
        res.json({ files }); // JSON object with a "files" array
    }
    catch (err) {
        console.error(err);
        res.status(500).json({ error: 'Error reading files' });
    }
});
app.use('/samples', express_1.default.static(SAMPLE_DIR));
app.get('/app/*', (req, res) => {
    res.sendFile(path_1.default.join(WEBAPP_DIR, 'index.html'));
});
app.listen(port, () => {
    console.log(`Server running at http://localhost:${port}`);
    console.log(`Static files served at http://localhost:${port}/samples`);
});

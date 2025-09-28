import path from "path";
import fs from "fs";
import dotenv from "dotenv";
import express from "express";
import multer from "multer";


const app = express();
const port = 3001;

dotenv.config();

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

const storage = multer.diskStorage({
  destination: (req, file, cb) => {
    const folder = req.query.folder as string || '';
    const uploadPath = path.join(SAMPLE_DIR!, folder);
    fs.mkdirSync(uploadPath, { recursive: true });
    cb(null, uploadPath);
  },
  filename: (req, file, cb) => {
    cb(null, file.originalname);
  }
});


const upload = multer({ storage });

// Recursive function to list all files relative to SAMPLE_DIR
function listFilesRelative(dir: string, baseDir: string = dir): string[] {
  let results: string[] = [];
  const entries = fs.readdirSync(dir, { withFileTypes: true });

  for (const entry of entries) {
    const fullPath = path.join(dir, entry.name);
    if (entry.isDirectory()) {
      results = results.concat(listFilesRelative(fullPath, baseDir));
    } else {
      results.push(path.relative(baseDir, fullPath));
    }
  }

  return results;
}

// List files endpoint
app.get('/files', (req, res) => {
  try {
    const files = listFilesRelative(SAMPLE_DIR);
    const detailedFiles = files.map((relativePath) => {
      const fullPath = path.join(SAMPLE_DIR, relativePath);
      const stat = fs.statSync(fullPath);

      return {
        path: relativePath,
        size: stat.size, // bytes
      };
    });

    res.json({ files: detailedFiles });
  } catch (err) {
    console.error(err);
    res.status(500).json({ error: 'Error reading files' });
  }
});

// File upload API
// Use POST /upload with form-data field "file"
app.post('/upload', upload.single('file'), (req, res) => {
  if (!req.file) {
    return res.status(400).json({ error: 'No file uploaded' });
  }

  res.json({
    message: 'File uploaded successfully',
    file: {
      filename: req.file.filename,
      size: req.file.size,
      path: req.file.path
    }
  });
});

// Serve static files
app.use("/samples", express.static(SAMPLE_DIR));
app.use('/app', express.static(WEBAPP_DIR));

// SPA routes
app.get('/app', (req, res) => {
  res.sendFile('index.html', { root: WEBAPP_DIR });
});
app.get('/app/', (req, res) => {
  res.sendFile('index.html', { root: WEBAPP_DIR });
});

app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
  console.log(`Static files served at http://localhost:${port}/samples`);
});

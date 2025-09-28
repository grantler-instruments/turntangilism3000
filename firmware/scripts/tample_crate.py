import os
import json
import cgi
from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib import parse

SAMPLE_DIR = "/app/samples"
SAMPLE_DIR = "/root/Bela/samples"

PORT = 3001

def list_files_recursive(base_dir):
    items = []

    for root, dirs, files in os.walk(base_dir):
        # Add directories
        for d in dirs:
            dir_path = os.path.join(root, d)
            rel_path = os.path.relpath(dir_path, base_dir)
            # Check if directory is empty
            if not os.listdir(dir_path):
                items.append({"path": rel_path, "type": "directory", "empty": True})
            else:
                items.append({"path": rel_path, "type": "directory", "empty": False})

        # Add files
        for f in files:
            full_path = os.path.join(root, f)
            rel_path = os.path.relpath(full_path, base_dir)
            size = os.path.getsize(full_path)
            items.append({"path": rel_path, "type": "file", "size": size})

    return items


class RequestHandler(BaseHTTPRequestHandler):

    def _set_headers(self, content_type="application/json", extra_headers=None):
        self.send_header("Content-Type", content_type)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        if extra_headers:
            for k, v in extra_headers.items():
                self.send_header(k, v)
        self.end_headers()

    def do_OPTIONS(self):
        self.send_response(200)
        self._set_headers()

    def do_GET(self):
        if self.path == "/files":
            try:
                files = list_files_recursive(SAMPLE_DIR)
                self.send_response(200)
                self._set_headers()
                self.wfile.write(json.dumps({"files": files}).encode("utf-8"))
            except Exception as e:
                self.send_response(500)
                self._set_headers()
                self.wfile.write(json.dumps({"error": str(e)}).encode("utf-8"))

        elif self.path.startswith("/samples"):
            self.serve_file(SAMPLE_DIR)

        else:
            self.send_response(404)
            self._set_headers()

    def do_POST(self):
        if self.path.startswith("/upload"):
            folder = self.path[len("/upload"):].lstrip("/")
            upload_path = os.path.join(SAMPLE_DIR, folder)
            if not os.path.exists(upload_path):
                os.makedirs(upload_path)

            ctype = self.headers.get('content-type')
            if not ctype or 'multipart/form-data' not in ctype:
                self.send_response(400)
                self._set_headers()
                return

            try:
                fs = cgi.FieldStorage(
                    fp=self.rfile,
                    headers=self.headers,
                    environ={'REQUEST_METHOD':'POST',
                            'CONTENT_TYPE': self.headers['Content-Type']},
                )
                uploaded_files = []

                for key in fs.keys():
                    field_item = fs[key]
                    if isinstance(field_item, list):
                        items = field_item
                    else:
                        items = [field_item]

                    for item in items:
                        filename = item.filename
                        filedata = item.file.read()
                        filepath = os.path.join(upload_path, filename)
                        with open(filepath, "wb") as f:
                            f.write(filedata)
                        uploaded_files.append({"filename": filename, "path": filepath, "size": len(filedata)})

                self.send_response(200)
                self._set_headers()
                self.wfile.write(json.dumps({
                    "message": "Files uploaded successfully",
                    "folder": folder,
                    "files": uploaded_files
                }).encode("utf-8"))

            except Exception as e:
                self.send_response(500)
                self._set_headers()
                self.wfile.write(json.dumps({"error": str(e)}).encode("utf-8"))
        else:
            self.send_response(404)
            self._set_headers()


    def serve_file(self, base_dir):
        rel_path = parse.unquote(self.path.lstrip("/samples"))
        full_path = os.path.join(base_dir, rel_path)
        if os.path.isdir(full_path):
            self.send_response(403)
            self._set_headers()
            return
        if os.path.exists(full_path):
            self.send_response(200)
            self._set_headers("application/octet-stream")
            with open(full_path, "rb") as f:
                self.wfile.write(f.read())
        else:
            self.send_response(404)
            self._set_headers()

httpd = HTTPServer(("0.0.0.0", PORT), RequestHandler)
print("Server running on port", PORT)
httpd.serve_forever()

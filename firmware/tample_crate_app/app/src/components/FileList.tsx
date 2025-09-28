import {
  Accordion,
  AccordionSummary,
  AccordionDetails,
  Typography,
  Box,
} from "@mui/material";
import ExpandMoreIcon from "@mui/icons-material/ExpandMore";
import FileItem from "./FileItem";
import { useDropzone } from "react-dropzone";

type FileEntry = {
  path: string;
  size: number;
};

type DirectoryEntry = {
  path: string;
  empty: boolean;
};

type FileListProps = {
  files: FileEntry[];
  directories: DirectoryEntry[];
};

// Natural sort helper
function naturalCompare(a: string, b: string) {
  const ax = a.match(/\d+|\D+/g) || [];
  const bx = b.match(/\d+|\D+/g) || [];

  for (let i = 0; i < Math.min(ax.length, bx.length); i++) {
    const aPart = ax[i];
    const bPart = bx[i];

    if (aPart !== bPart) {
      const aNum = parseInt(aPart, 10);
      const bNum = parseInt(bPart, 10);

      if (!isNaN(aNum) && !isNaN(bNum)) {
        return aNum - bNum; // numeric comparison
      } else {
        return aPart.localeCompare(bPart); // string comparison
      }
    }
  }

  return ax.length - bx.length;
}

export default function FileList({ files, directories }: FileListProps) {
  // Group files and directories by top-level folder
  const grouped: Record<string, { files: FileEntry[]; hasEmptyFolder: boolean }> = {};

  // Mark empty top-level folders
  directories.forEach((dir) => {
    const parts = dir.path.split("/");
    const folder = parts[0] || "root";
    if (!grouped[folder]) grouped[folder] = { files: [], hasEmptyFolder: false };
    if (dir.empty) grouped[folder].hasEmptyFolder = true;
  });

  // Group files
  files.forEach((file) => {
    const parts = file.path.split("/");
    const folder = parts[0] || "root";
    if (!grouped[folder]) grouped[folder] = { files: [], hasEmptyFolder: false };
    grouped[folder].files.push(file);
  });

  // Sort each folder's files naturally
  Object.keys(grouped).forEach((folder) => {
    grouped[folder].files.sort((a, b) => {
      const aName = a.path.split("/").pop()!;
      const bName = b.path.split("/").pop()!;
      return naturalCompare(aName, bName);
    });
  });

  const uploadFiles = async (folder: string, uploadFiles: File[]) => {
    const formData = new FormData();
    uploadFiles.forEach((file) => formData.append("file", file));

    try {
      const serverUrl = import.meta.env.VITE_SERVER_URL || "http://localhost:3001";
      const res = await fetch(`${serverUrl}/upload/${encodeURIComponent(folder)}`, {
        method: "POST",
        body: formData,
      });

      if (!res.ok) throw new Error("Upload failed");
      const data = await res.json();
      console.log("Upload success:", data);
      alert(`Uploaded ${uploadFiles.length} file(s) to folder "${folder}"`);
    } catch (err) {
      console.error(err);
      alert("Upload failed");
    }
  };

  return (
    <div>
      {Object.entries(grouped).map(([folder, { files, hasEmptyFolder }]) => {
        const onDrop = (acceptedFiles: File[]) => {
          uploadFiles(folder, acceptedFiles);
        };

        const { getRootProps, getInputProps, isDragActive } = useDropzone({ onDrop });

        return (
          <Accordion key={folder}>
            <AccordionSummary expandIcon={<ExpandMoreIcon />}>
              <Typography>{folder}</Typography>
            </AccordionSummary>
            <AccordionDetails>
              {/* Dropzone */}
              <Box
                {...getRootProps()}
                sx={{
                  border: "2px dashed gray",
                  borderRadius: 1,
                  padding: 2,
                  textAlign: "center",
                  mb: 2,
                  backgroundColor: isDragActive ? "#f0f0f0" : "transparent",
                  cursor: "pointer",
                }}
              >
                <input {...getInputProps()} />
                {isDragActive
                  ? "Drop the files here..."
                  : "Drag & drop files here, or click to select"}
              </Box>

              {/* File list */}
              {files.length === 0 && hasEmptyFolder ? (
                <p style={{ fontStyle: "italic" }}>This folder is empty</p>
              ) : (
                <ul style={{ listStyle: "none", paddingLeft: 0 }}>
                  {files.filter(f => f.path.endsWith(".wav")).map((f) => (
                    <FileItem key={f.path} {...f} />
                  ))}
                </ul>
              )}
            </AccordionDetails>
          </Accordion>
        );
      })}
    </div>
  );
}

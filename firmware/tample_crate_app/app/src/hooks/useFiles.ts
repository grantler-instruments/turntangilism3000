import { useEffect, useState } from "react";
import { fetchFiles as fetchFilesApi } from "../api/files";

type FileEntry = {
  path: string;
  size: number;
};

type DirectoryEntry = {
  path: string;
  empty: boolean;
};

export function useFiles() {
  const [files, setFiles] = useState<FileEntry[]>([]);
  const [directories, setDirectories] = useState<DirectoryEntry[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    fetchFilesApi()
      .then(({ files, directories }) => {
        setFiles(files);
        setDirectories(directories);
        setLoading(false);
      })
      .catch((err) => {
        setError(err.message);
        setLoading(false);
      });
  }, []);

  return { files, directories, loading, error };
}

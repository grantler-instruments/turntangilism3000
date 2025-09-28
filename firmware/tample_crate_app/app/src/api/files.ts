export async function fetchFiles() {
  const serverUrl = import.meta.env.VITE_SERVER_URL || "http://localhost:3001";
  const response = await fetch(`${serverUrl}/files`);
  if (!response.ok) throw new Error("Failed to fetch files");

  const data = await response.json();

  const files = data.files.filter((f: any) => f.type === "file");
  const directories = data.files.filter((f: any) => f.type === "directory");

  return { files, directories };
}

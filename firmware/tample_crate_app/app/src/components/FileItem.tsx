type FileItemProps = {
  path: string;
  size?: number; // optional
};

export default function FileItem({ path, size }: FileItemProps) {
  const serverUrl = import.meta.env.VITE_SERVER_URL || "http://localhost:3001";
  const fileUrl = `${serverUrl}/samples/${path}`;

  return (
    <li style={{ marginBottom: "8px" }}>
      <span>{path.split("/").pop()}</span>{" "}
      {size !== undefined && (
        <span style={{ color: "gray", fontSize: "0.9em" }}>
          ({(size / 1024).toFixed(1)} KB)
        </span>
      )}
      <audio
        src={fileUrl}
        controls
        style={{ display: "block", marginTop: "4px" }}
      />
    </li>
  );
}

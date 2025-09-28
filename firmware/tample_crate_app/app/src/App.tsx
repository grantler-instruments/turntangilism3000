import { useFiles } from "./hooks/useFiles";
import FileList from "./components/FileList";
import { Box, Typography } from "@mui/material";

function App() {
  const { files, directories, loading, error } = useFiles();

  if (loading) return <p>Loading...</p>;
  if (error) return <p>Error: {error}</p>;

  return (
    <Box
      display={"flex"}
      flexDirection={"column"}
      padding={2}
    >
      <Box marginBottom={"24px"}>
      <Typography variant="h1">Tample Crate</Typography>
      <Typography variant="body1">Dig your tamples, find your sounds</Typography>
      </Box>
      <FileList files={files} directories={directories} />;
    </Box>
  );
}

export default App;

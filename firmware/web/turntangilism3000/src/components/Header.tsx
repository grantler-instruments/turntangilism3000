import { Box, Button, MenuItem, Select } from "@mui/material";
import useStore from "../store/store";

const Header = () => {
  const active = useStore((state) => state.active);
  const toggle = useStore((state) => state.toggle);
  const sampleBank = useStore((state) => state.sampleBank);
  const setSampleBank = useStore((state) => state.setSampleBank);
  return (
    <Box display={"flex"}>
      <Button variant="outlined" onClick={toggle}>
        {active && <>stop</>}
        {!active && <>start</>}
      </Button>
      <Box flex={1}></Box>
      <Box>
        <Select
          value={sampleBank}
          label="Sample bank"
          onChange={(event) => {
            setSampleBank(event.target.value)
          }}
        >
          {[...Array(16)].map((_, i) => (
            <MenuItem key={i + 1} value={i + 1}>
              {i + 1}
            </MenuItem>
          ))}
        </Select>
      </Box>
    </Box>
  );
};
export default Header;

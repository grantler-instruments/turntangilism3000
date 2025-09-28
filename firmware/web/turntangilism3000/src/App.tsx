import { useEffect, useState } from "react";
import useStore from "./store/store";
import { Box, Button, Slider, TextField, ToggleButton } from "@mui/material";
import Header from "./components/Header";

const getColor = (note: number) => {
  let fill = "green";
  switch (note) {
    case 1: {
      fill = "green";
      break;
    }
    case 2: {
      fill = "red";
      break;
    }
    case 3: {
      fill = "brown";
      break;
    }
    case 4: {
      fill = "yellow";
      break;
    }
    case 5: {
      fill = "white";
      break;
    }
    case 6: {
      fill = "blue";
      break;
    }
    case 7: {
      fill = "orange";
      break;
    }
  }
  return fill;
};

function App() {
  const rotation = useStore((state) => state.rotation);
  const tokens = useStore((state) => state.tokens);
  const removeToken = useStore((state) => state.removeToken);
  const addToken = useStore((state) => state.addToken);
  const toggle = useStore((state) => state.toggle);
  const setRpm = useStore((state) => state.setRpm);
  const rpm = useStore((state) => state.rpm);
  const [note, setNote] = useState(1);

  const [track, setTrack] = useState(0);

  useEffect(() => {
    toggle();
  }, []);

  const handleClick = (event: any, i: number) => {
    // Get the bounding box of the SVG
    const { left, top, width, height } = event.target.getBoundingClientRect();

    // Calculate the click's relative position
    const x = event.clientX - (left + width / 2); // Center X
    const y = event.clientY - (top + height / 2); // Center Y

    // Calculate the angle in radians
    let angle = Math.atan2(y, x);

    // Convert the angle to degrees and offset for 12 o'clock
    let degrees = (angle * (180 / Math.PI) + 90) % 360;
    if (degrees < 0) degrees += 360;

    addToken(note, degrees);
  };

  return (
    <Box display={"flex"} flexDirection={"column"}>
      <Header></Header>
      <Box display={"flex"} gap={3}>
        <svg
          xmlns="http://www.w3.org/2000/svg"
          viewBox="-170 -170 340 340"
          width="340"
          height="340"
        >
          <circle
            cx="0"
            cy="0"
            r="170"
            fill="none"
            stroke="black"
            strokeWidth="1"
          />
          <rect
            x="-2"
            y="-160"
            width="4"
            height="140"
            fill="red"
            transform={`rotate(${rotation})`}
          />

          <circle
            key={`circle-7`}
            cx="0"
            cy="0"
            r={22 + 7 * 20}
            strokeWidth="18"
            fill="none"
            stroke="white"
            onClick={(event) => {
              handleClick(event, 7);
            }}
          />

          <circle
            cx="0"
            cy="0"
            r="3.6"
            fill="black"
            stroke="black"
            strokeWidth="1"
          />

          {tokens.map((token, i) => {
            const { rotation } = token; // Token's track and rotation
            const radius = 22 + 7 * 20; // Calculate the radius based on track
            const x = radius * Math.cos(((rotation - 90) * Math.PI) / 180); // X-coordinate
            const y = radius * Math.sin(((rotation - 90) * Math.PI) / 180); // Y-coordinate

            return (
              <circle
                key={`token-${i}`}
                cx={x}
                cy={y}
                r="8" // Token size
                fill={getColor(token.note)}
                onClick={() => {
                  removeToken(token.id);
                }}
              />
            );
          })}
        </svg>
        <Box display={"flex"} flexDirection={"column"} gap={3}>
          <Box display={"flex"} flexDirection={"column"}>
            <Slider></Slider>
            <Box display={"flex"}>
              <ToggleButton
                onClick={() => {
                  setRpm(33);
                }}
                value={33}
                selected={rpm === 33}
              >
                33
              </ToggleButton>
              <ToggleButton
                value={33}
                selected={rpm === 45}
                onClick={() => {
                  setRpm(45);
                }}
              >
                45
              </ToggleButton>
            </Box>
          </Box>
          <Button
            variant="outlined"
            fullWidth
            onClick={() => {
              toggle();
            }}
          >
            toggle
          </Button>
          <Box>
            {Array(8)
              .fill(0)
              .map((_, i) => {
                return (
                  <Button
                    variant="outlined"
                    key={`note-${i}`}
                    onClick={() => setNote(i + 1)}
                    sx={{ backgroundColor: getColor(i + 1) }}
                  >
                    {i + 1}
                  </Button>
                );
              })}
          </Box>
        </Box>
      </Box>
    </Box>
  );
}

export default App;

import { createTheme } from '@mui/material/styles';
import { grey } from '@mui/material/colors';

const theme = createTheme({
  palette: {
    mode: 'dark',
    background: {
      default: '#000000', // Pure black
      paper: '#121212',   // Slightly lighter black for paper elements
    },
    primary: {
      main: '#90caf9',    // Light blue as primary color for good contrast
    },
    secondary: {
      main: '#f48fb1',    // Pink as secondary color
    },
    text: {
      primary: '#ffffff', // White text
      secondary: grey[400], // Light gray for secondary text
    },
    divider: grey[800],   // Dark gray divider
  },
  typography: {
    fontFamily: '"Roboto", "Helvetica", "Arial", sans-serif',
    h1: {
      color: '#ffffff',
    },
    h2: {
      color: '#ffffff',
    },
    h3: {
      color: '#ffffff',
    },
    h4: {
      color: '#ffffff',
    },
    h5: {
      color: '#ffffff',
    },
    h6: {
      color: '#ffffff',
    },
    subtitle1: {
      color: grey[400],
    },
    subtitle2: {
      color: grey[400],
    },
    body1: {
      color: '#ffffff',
    },
    body2: {
      color: grey[400],
    },
  },
  components: {
    MuiAppBar: {
      styleOverrides: {
        root: {
          backgroundColor: '#121212',
        },
      },
    },
    MuiButton: {
      styleOverrides: {
        root: {
          textTransform: 'none', // Optional: removes uppercase transformation
        },
      },
    },
    MuiPaper: {
      styleOverrides: {
        root: {
          backgroundColor: '#121212',
        },
      },
    },
  },
});

export default theme;
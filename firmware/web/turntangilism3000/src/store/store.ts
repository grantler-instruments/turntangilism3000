import { create } from "zustand";
import { createJSONStorage, devtools, persist } from "zustand/middleware";
import packageJson from "../../package.json";
import { v4 as uuidv4 } from "uuid";
import { WebMidi } from "webmidi";

WebMidi.enable()
  .then(onEnabled)
  .catch((err) => alert(err));

function onEnabled() {
  // Inputs
  WebMidi.inputs.forEach((input) =>
    console.log(input.manufacturer, input.name)
  );

  // Outputs
  WebMidi.outputs.forEach((output) =>
    console.log(output.manufacturer, output.name)
  );
}

document.addEventListener("keydown", (event)=> {
  if(event.key === " "){
    useStore.getState().toggle()
  }
})

interface State {
  version: string;
  active: boolean;
  sampleBank: number;
  rpm: number;
  tokens: any[];
  rotation: number;
  toggle: () => void;
  addToken: (note: number, rotation: number) => void;
  removeToken: (id: string) => void;
  setRpm: (rpm: number) => void;
  setSampleBank: (sampleBank: number) => void;
}

let intervalId: any = null;

const useStore = create<State>()(
  devtools(
    persist(
      (set, get) => ({
        version: packageJson.version,
        active: false,
        rpm: 33,
        rotation: 0,
        sampleBank: 1,
        tokens: [
          {
            id: uuidv4(),
            rotation: 90,
            note: 1,
          },
          {
            id: uuidv4(),
            rotation: 0,
            note: 1,
          },
          {
            id: uuidv4(),
            rotation: 180,
            note: 1,
          },
          {
            id: uuidv4(),
            rotation: 200,
            note: 1,
          },
        ],
        toggle: () => {
          const { active } = get();
          if (intervalId) {
            clearInterval(intervalId);
            intervalId = null;
            set({ active: false });
          } else {
            const { rpm } = get();
            const intervalTime = (60 / (rpm * 360)) * 1000;

            intervalId = setInterval(() => {
              const { sampleBank } = get();
              const newRotation = (get().rotation + 1) % 360;
              get().tokens.forEach((token) => {
                if (token.rotation === newRotation) {
                  WebMidi.outputs
                    .filter((output) => output.name.includes("IAC"))
                    .forEach((output) => {
                      output.channels[sampleBank].playNote(token.note, {
                        duration: 100,
                      });
                    });
                }
              });
              set((state) => ({ rotation: newRotation }));
            }, intervalTime);
            set({ active: true });
          }
        },
        addToken(note, rotation) {
          set((state) => ({
            tokens: [
              ...state.tokens,
              {
                id: uuidv4(),
                note,
                rotation: Math.round(rotation),
              },
            ],
          }));
        },
        removeToken(id) {
          set((state) => ({
            tokens: state.tokens.filter((token) => token.id !== id),
          }));
        },
        setRpm(rpm) {
          set({ rpm });
        },
        setSampleBank(sampleBank) {
          set({ sampleBank });
        },
      }),
      {
        name: "app",
        storage: createJSONStorage(() => localStorage),
        partialize: (state) => {},
      }
    ),
    { name: "app" }
  )
);

export default useStore;

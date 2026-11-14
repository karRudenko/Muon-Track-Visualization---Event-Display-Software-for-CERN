// Shared constants: colours, paths and defaults.

export const DEG = Math.PI / 180;

// Colour of each strip projection (x, y, u, v).
export const COLORS = { x: 0x58a6ff, y: 0x3fb950, u: 0xd29922, v: 0xbc8cff };
export const TRACK_COLOR = 0xffd33d;
export const VERTEX_COLOR = 0xff4d4d;
export const TARGET_COLOR = 0xff8c1a;

// z is compressed by default.
export const DEFAULT_Z_SCALE = 0.1;

// Where the data lives.
export const PATHS = {
  geometry: "data/geometry.json",
  // live mode: served by scripts/serve.py
  apiInfo: "api/info",
  apiEvent: id => `api/event/${id}`,
  // static mode: pre-exported files
  staticIndex: "data/events/index.json",
  staticEvent: id => `data/events/event_${id}.json`,
};

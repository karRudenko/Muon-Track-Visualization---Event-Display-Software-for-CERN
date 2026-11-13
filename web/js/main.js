import { createViewer } from "./scene.js";
import { drawGeometry } from "./detector.js";
import { drawEvent, summarize } from "./eventView.js";
import { connect, loadGeometry } from "./dataSource.js";
import { createUI } from "./ui.js";

const viewer = createViewer(document.body);
let source = null;
let geometry = null;
let currentId = null;
let requestSeq = 0;

const ui = createUI({
  onGoto: id => gotoEvent(id),
  onStep: delta => source && gotoEvent(source.catalog.step(currentId, delta)),
  onRandom: () => source && gotoEvent(source.catalog.random(currentId)),
  onZScale: s => viewer.setZScale(s),
  onLayerVisible: (name, visible) => { viewer.layers[name].visible = visible; },
});

// ---------- URL: ?event=<number> makes a shareable link ----------
function eventFromUrl() {
  const raw = new URLSearchParams(location.search).get("event");
  return raw !== null && /^\d+$/.test(raw) ? Number(raw) : null;
}
function writeUrl(id) {
  const url = new URL(location.href);
  url.searchParams.set("event", id);
  history.replaceState(null, "", url);
}

// ---------- showing events ----------
function render(ev) {
  const skipped = drawEvent(viewer.layers, ev, geometry);
  ui.setInfo(summarize(ev, skipped));
}

async function gotoEvent(id) {
  if (!source) return;
  const { catalog } = source;
  if (!catalog.has(id)) {
    ui.setInfo(`Event ${Number.isNaN(id) ? "?" : id} is not available.\nValid events: ${catalog.describe()}`, true);
    return;
  }
  const seq = ++requestSeq;
  ui.setBusy(true);
  try {
    const ev = await source.getEvent(id);
    if (seq !== requestSeq) return;
    render(ev);
    currentId = id;
    ui.setCurrentEvent(id);
    writeUrl(id);
  } catch (e) {
    if (seq === requestSeq) ui.setInfo(`Could not load event ${id}:\n${e.message}`, true);
  } finally {
    if (seq === requestSeq) ui.setBusy(false);
  }
}

// ---------- start-up ----------
const STARTUP_HELP = `
Open the page through the project server (not by double-clicking index.html):
  make serve           live access to a ROOT file
  make serve-static    only the events already exported
then open http://localhost:8000`;

async function init() {
  ui.syncZScale();
  try {
    [source, geometry] = await Promise.all([connect(), loadGeometry()]);
  } catch (e) {
    ui.setSource("not connected");
    ui.setInfo(`Could not load data:\n${e.message}\n${STARTUP_HELP}`, true);
    return;
  }
  viewer.setZCenter(drawGeometry(viewer.layers, geometry));
  ui.setCatalog(source.catalog);

  if (source.catalog.size === 0) {
    ui.setInfo("No events available.\nExport some with `make events` or start the live server with `make serve`.", true);
    return;
  }
  const fromUrl = eventFromUrl();
  if (fromUrl !== null && !source.catalog.has(fromUrl)) console.warn(`?event=${fromUrl} is not available`);
  await gotoEvent(fromUrl !== null && source.catalog.has(fromUrl) ? fromUrl : source.catalog.random());
}

init();

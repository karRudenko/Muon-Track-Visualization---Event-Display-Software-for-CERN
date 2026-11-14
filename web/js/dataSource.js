// Where events come from. Two interchangeable sources:
//   LiveSource - scripts/serve.py exports any event of a ROOT file on demand (api/...)
//   StaticSource - pre-exported files listed in data/events/index.json
// connect() picks the live API if it is there and falls back to static files.

import { PATHS } from "./config.js";

async function fetchJson(url) {
  const r = await fetch(url, { cache: "no-store" });
  if (!r.ok) {
    let detail = "";
    try { detail = (await r.json()).error ?? ""; } catch { }
    throw new Error(`${url}: HTTP ${r.status}${detail ? " - " + detail : ""}`);
  }
  return r.json();
}

// The set of event numbers the user can pick from.
export class EventCatalog {
  #n; #ids;

  static range(n) { return new EventCatalog(n, null); }
  static list(ids) { return new EventCatalog(0, [...new Set(ids)].sort((a, b) => a - b)); }

  constructor(n, ids) { this.#ids = ids; this.#n = ids ? ids.length : n; }

  get size() { return this.#n; }
  get isRange() { return this.#ids === null; }
  get ids() { return this.#ids ?? []; }

  idAt(i) { return this.#ids ? this.#ids[i] : i; }
  indexOf(id) {
    if (this.#ids) return this.#ids.indexOf(id);
    return Number.isInteger(id) && id >= 0 && id < this.#n ? id : -1;
  }
  has(id) { return this.indexOf(id) >= 0; }

  describe() {
    if (this.#n === 0) return "no events";
    return this.isRange ? `0 - ${this.#n - 1}` : this.#ids.join(", ");
  }

  // Next/previous event with wrap-around. An id that is not in the catalog
  step(id, delta) {
    const i = this.indexOf(id);
    const base = i >= 0 ? i : (delta > 0 ? -1 : 0);
    return this.idAt((((base + delta) % this.#n) + this.#n) % this.#n);
  }

  // Uniformly random event, different from `exceptId` when there is a choice.
  random(exceptId = null) {
    const n = this.#n;
    if (n === 0) return null;
    const ex = exceptId === null ? -1 : this.indexOf(exceptId);
    if (n === 1 || ex < 0) return this.idAt(Math.floor(Math.random() * n));
    let i = Math.floor(Math.random() * (n - 1));
    if (i >= ex) i++;
    return this.idAt(i);
  }
}

class LiveSource {
  constructor(info) {
    this.catalog = EventCatalog.range(info.nEvents);
  }
  getEvent(id) { return fetchJson(PATHS.apiEvent(id)); }
}

class StaticSource {
  constructor(ids) {
    this.catalog = EventCatalog.list(ids);
  }
  getEvent(id) { return fetchJson(PATHS.staticEvent(id)); }
}

export async function connect() {
  try {
    return new LiveSource(await fetchJson(PATHS.apiInfo));
  } catch {}
  const index = await fetchJson(PATHS.staticIndex);
  return new StaticSource(index.events ?? []);
}

export const loadGeometry = () => fetchJson(PATHS.geometry);

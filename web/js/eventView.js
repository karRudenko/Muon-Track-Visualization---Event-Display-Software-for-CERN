// Draws one event and builds its text summary.

import * as THREE from "three";
import { COLORS, DEG, TRACK_COLOR, VERTEX_COLOR } from "./config.js";
import { clearGroup } from "./scene.js";

function tube(a, b, radius, color) {
  const geom = new THREE.TubeGeometry(new THREE.LineCurve3(a, b), 1, radius, 8, false);
  return new THREE.Mesh(geom, new THREE.MeshBasicMaterial({ color }));
}

// Throws if `ev` does not look like an exported event.
export function assertEvent(ev) {
  for (const key of ["hits", "tracks", "vertices"]) {
    if (!Array.isArray(ev?.[key])) throw new Error(`not an event file: missing "${key}" array`);
  }
}

// Returns how many hits could not be matched to a module of the geometry.
export function drawEvent(layers, ev, geometry) {
  assertEvent(ev);
  clearGroup(layers.hits);
  clearGroup(layers.tracks);
  clearGroup(layers.vertices);

  // Each hit is ONE measurement (a strip): a segment across the measurement direction at the hit's z.
  let skipped = 0;
  for (const h of ev.hits) {
    const m = geometry[h.station]?.modules[h.module];
    if (!m) { skipped++; continue; }
    const phi = m.angle * DEG, half = m.sizeAlongStrips / 2, c = h.coord;
    const mx = Math.cos(phi), my = Math.sin(phi);     // measurement direction
    const sx = -Math.sin(phi), sy = Math.cos(phi);    // direction along the strips
    layers.hits.add(tube(
      new THREE.Vector3(c * mx - half * sx, c * my - half * sy, h.z),
      new THREE.Vector3(c * mx + half * sx, c * my + half * sy, h.z),
      0.04, COLORS[m.projection]));
  }

  for (const t of ev.tracks) {
    layers.tracks.add(tube(new THREE.Vector3(...t.p1), new THREE.Vector3(...t.p2), 0.05, TRACK_COLOR));
  }

  for (const v of ev.vertices) {
    const s = new THREE.Mesh(new THREE.SphereGeometry(0.25, 16, 12),
      new THREE.MeshBasicMaterial({ color: VERTEX_COLOR }));
    s.position.set(...v.pos);
    layers.vertices.add(s);
  }
  return skipped;
}

export function summarize(ev, skippedHits = 0) {
  const lines = [
    `Event ${ev.event}`,
    `hits: ${ev.hits.length}   tracks: ${ev.tracks.length}   vertices: ${ev.vertices.length}`,
  ];
  ev.vertices.forEach((v, i) => lines.push(
    `vertex ${i}: z=${v.pos[2].toFixed(1)} cm, θμ=${(v.muonTheta * 1e3).toFixed(3)} mrad, ` +
    `θe=${(v.electronTheta * 1e3).toFixed(2)} mrad`));
  if (skippedHits) lines.push(`⚠ hits outside the geometry: ${skippedHits}`);
  return lines.join("\n");
}

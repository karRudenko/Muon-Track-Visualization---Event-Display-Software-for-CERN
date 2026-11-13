// Draws one event and builds its text summary.

import * as THREE from "three";
import { COLORS, DEG, TRACK_COLOR, VERTEX_COLOR } from "./config.js";
import { clearGroup } from "./scene.js";
import { trackExtent } from "./trackExtent.js";

function tube(a, b, radius, color) {
  const geom = new THREE.TubeGeometry(new THREE.LineCurve3(a, b), 1, radius, 8, false);
  return new THREE.Mesh(geom, new THREE.MeshBasicMaterial({ color }));
}
 

function pointAt(t, z) {
  const [x1, y1, z1] = t.p1, [x2, y2, z2] = t.p2;
  const f = (z - z1) / (z2 - z1);
  return new THREE.Vector3(x1 + (x2 - x1) * f, y1 + (y2 - y1) * f, z);
}

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

  // Each hit: a segment across the measurement direction at the hit's z.
  let skipped = 0;
  for (const h of ev.hits) {
    const m = geometry[h.station]?.modules[h.module];
    if (!m) { skipped++; continue; }
    const phi = m.angle * DEG, half = m.sizeAlongStrips / 2, c = h.coord;
    const mx = Math.cos(phi), my = Math.sin(phi);
    const sx = -Math.sin(phi), sy = Math.cos(phi);
    layers.hits.add(tube(
      new THREE.Vector3(c * mx - half * sx, c * my - half * sy, h.z),
      new THREE.Vector3(c * mx + half * sx, c * my + half * sy, h.z),
      0.04, COLORS[m.projection]));
  }

  // a track is drawn only between its first and last hit; a new track starts at its vertex
  for (const t of ev.tracks) {
    const ext = trackExtent(t, ev.hits, geometry, ev.vertices);
    const a = ext ? pointAt(t, ext.zStart) : new THREE.Vector3(...t.p1);
    const b = ext ? pointAt(t, ext.zEnd) : new THREE.Vector3(...t.p2);
    layers.tracks.add(tube(a, b, 0.05, TRACK_COLOR));
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

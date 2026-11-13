// Finds the part of a track that really exists, using the hits that lie on it.

import { DEG } from "./config.js";

const MAX_DISTANCE = 0.15; 
const MIN_MODULES = 2 / 3; 
const VERTEX_DISTANCE = 0.25; 

// Returns { zStart, zEnd }, or null if the track has no hits.
export function trackExtent(track, hits, geometry, vertices = []) {
  const [x1, y1, z1] = track.p1;
  const [x2, y2, z2] = track.p2;
  if (z1 === z2) return null;
  const slopeX = (x2 - x1) / (z2 - z1);
  const slopeY = (y2 - y1) / (z2 - z1);

  // for every module: z of the hit closest to the track
  const nearest = geometry.map(st => st.modules.map(() => ({ dist: MAX_DISTANCE, z: null })));
  for (const h of hits) {
    const m = geometry[h.station]?.modules[h.module];
    if (!m) continue;
    const x = x1 + slopeX * (h.z - z1);
    const y = y1 + slopeY * (h.z - z1);
    const phi = m.angle * DEG;
    const dist = Math.abs(x * Math.cos(phi) + y * Math.sin(phi) - h.coord);
    const slot = nearest[h.station][h.module];
    if (dist < slot.dist) { slot.dist = dist; slot.z = h.z; }
  }

  // stations in which the track has hits
  const zs = nearest.map(mods => mods.filter(s => s.z !== null).map(s => s.z));
  const inStation = zs.map((z, i) => z.length >= Math.ceil(geometry[i].modules.length * MIN_MODULES));
  const first = inStation.indexOf(true);
  if (first < 0) return null;
  const last = inStation.lastIndexOf(true);

  let zStart = Math.min(...zs[first]);
  const target = geometry[first].target;
  if (target && target.z < zStart) {
    const prev = geometry[first - 1];
    const zMin = prev ? Math.max(...prev.modules.map(m => m.z)) : -Infinity;
    zStart = originVertexZ(vertices, zMin, zStart, z => [x1 + slopeX * (z - z1), y1 + slopeY * (z - z1)]) ?? target.z;
  }
  return { zStart, zEnd: Math.max(...zs[last]) };
}

function originVertexZ(vertices, zMin, zMax, lineAt) {
  let bestZ = null, bestDist = VERTEX_DISTANCE;
  for (const v of vertices) {
    const [vx, vy, vz] = v.pos;
    if (vz <= zMin || vz >= zMax) continue;
    const [x, y] = lineAt(vz);
    const dist = Math.hypot(x - vx, y - vy);
    if (dist < bestDist) { bestDist = dist; bestZ = vz; }
  }
  return bestZ;
}
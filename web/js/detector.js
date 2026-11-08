// Draws the detector from geometry.json.

import * as THREE from "three";
import { COLORS, DEG, TARGET_COLOR } from "./config.js";
import { clearGroup } from "./scene.js";

// Returns the z coordinate of the middle of the detector.
export function drawGeometry(group, geometry) {
  clearGroup(group);
  let zLo = Infinity, zHi = -Infinity;

  for (const st of geometry) {
    for (const m of st.modules) {
      zLo = Math.min(zLo, m.z);
      zHi = Math.max(zHi, m.z);
      const color = COLORS[m.projection];

      // module-local frame: x = measurement direction, y = along the strips, z = along the beam
      const grp = new THREE.Group();
      grp.position.z = m.z;
      grp.rotation.z = m.angle * DEG;

      const box = new THREE.BoxGeometry(m.sizeMeasurement, m.sizeAlongStrips, 0.4);
      const body = new THREE.Mesh(box, new THREE.MeshBasicMaterial(
        { color, transparent: true, opacity: 0.10, depthWrite: false }));
      const edges = new THREE.LineSegments(new THREE.EdgesGeometry(box),
        new THREE.LineBasicMaterial({ color, transparent: true, opacity: 0.55 }));

      // tilt = rotation around the strip axis; sign confirmed against data: hit_z = module_z - pos * sin(tilt)
      body.rotation.y = edges.rotation.y = m.tilt * DEG;
      grp.add(body, edges);
      group.add(grp);
    }

    if (st.target) {
      zLo = Math.min(zLo, st.target.z);
      zHi = Math.max(zHi, st.target.z);
      const t = new THREE.Mesh(
        new THREE.BoxGeometry(st.target.width, st.target.height, st.target.thickness),
        new THREE.MeshBasicMaterial({ color: TARGET_COLOR, transparent: true, opacity: 0.35, depthWrite: false }));
      t.position.z = st.target.z;
      group.add(t);
    }
  }
  return (zLo + zHi) / 2;
}

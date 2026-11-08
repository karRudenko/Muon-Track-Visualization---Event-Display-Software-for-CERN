// Three.js scene: renderer, camera, orbit controls and the four layers everything is drawn into.

import * as THREE from "three";
import { OrbitControls } from "three/addons/controls/OrbitControls.js";
import { DEFAULT_Z_SCALE } from "./config.js";

// Remove all children of a group and free their GPU resources.
export function clearGroup(group) {
  for (const child of [...group.children]) {
    group.remove(child);
    child.traverse(o => { o.geometry?.dispose(); o.material?.dispose(); });
  }
}

export function createViewer(container) {
  const scene = new THREE.Scene();
  const camera = new THREE.PerspectiveCamera(45, innerWidth / innerHeight, 0.1, 5000);
  camera.position.set(30, 18, 40);

  const renderer = new THREE.WebGLRenderer({ antialias: true });
  renderer.setPixelRatio(devicePixelRatio);
  renderer.setSize(innerWidth, innerHeight);
  container.appendChild(renderer.domElement);

  const controls = new OrbitControls(camera, renderer.domElement);
  controls.enableDamping = true;

  const world = new THREE.Group();
  scene.add(world);
  const layers = {
    geometry: new THREE.Group(),
    hits: new THREE.Group(),
    tracks: new THREE.Group(),
    vertices: new THREE.Group(),
  };
  Object.values(layers).forEach(g => world.add(g));

  let zScale = DEFAULT_Z_SCALE;
  let zCenter = 0;
  const applyZ = () => {
    world.scale.z = zScale;
    world.position.z = -zCenter * zScale;   // keep the detector centred on the origin
  };
  applyZ();

  addEventListener("resize", () => {
    camera.aspect = innerWidth / innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(innerWidth, innerHeight);
  });

  (function loop() {
    requestAnimationFrame(loop);
    controls.update();
    renderer.render(scene, camera);
  })();

  return {
    layers,
    setZScale(s) { zScale = s; applyZ(); },
    setZCenter(z) { zCenter = z; applyZ(); },
  };
}

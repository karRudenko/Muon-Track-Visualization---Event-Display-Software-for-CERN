#!/usr/bin/env python3
"""Convert a detector YAML configuration into web/data/geometry.json.

Usage:
    python3 scripts/geometry_to_json.py config/simple_3_stations.yaml [web/data/geometry.json]
"""
import argparse
import json
from pathlib import Path

import yaml

ROOT_DIR = Path(__file__).resolve().parent.parent
DEFAULT_OUTPUT = ROOT_DIR / "web" / "data" / "geometry.json"

# Measurement direction of each projection.
ANGLE = {"x": 0, "y": 90, "u": 135, "v": 45}


def build(yaml_path):
    """Return the list of stations described by the YAML file."""
    with open(yaml_path) as fh:
        cfg = yaml.safe_load(fh)

    mod_defaults = cfg["modulesConfiguration"]["geometry"]
    tgt_defaults = cfg["targetsConfiguration"]
    tilts = {k: v.get("tilt", 0.0) for k, v in cfg["detector"]["projections"].items()}

    stations = []
    for sid, st in enumerate(cfg["detector"]["stations"]):
        origin = st["origin"]

        target = None
        if "targetRelativePosition" in st:
            t = dict(tgt_defaults)
            tc = st.get("targetConfiguration", [])
            for item in (tc if isinstance(tc, list) else [tc]):
                t.update(item)
            target = {
                "z": origin + st["targetRelativePosition"],
                "width": t["width"],
                "height": t["height"],
                "thickness": t["thickness"],
            }

        modules = []
        for idx, m in enumerate(st["modules"]):
            g = dict(mod_defaults)
            g.update({k: v for k, v in m.items()
                      if k not in ("projection", "relativePosition", "hitResolution")})
            p = m["projection"]
            modules.append({
                "index": idx,
                "projection": p,
                "angle": ANGLE[p],
                "tilt": tilts[p],
                "z": origin + m["relativePosition"],
                "sizeMeasurement": g["sensorSizeMeasurementDirection"],
                "sizeAlongStrips": g["sensorSizePerpendicularDirection"],
                "thickness": g["sensorThickness"],
                "sensorDistance": g["distanceBetweenSensorCenters"],
                "hitResolution": m["hitResolution"],
            })
        stations.append({"id": sid, "origin": origin, "target": target, "modules": modules})
    return stations


def write(yaml_path, out_path):
    out_path = Path(out_path)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with open(out_path, "w") as fh:
        json.dump(build(yaml_path), fh, indent=2)
    return out_path


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("config", help="detector YAML file")
    ap.add_argument("output", nargs="?", default=str(DEFAULT_OUTPUT), help="output JSON (default: %(default)s)")
    args = ap.parse_args()
    print("written", write(args.config, args.output))


if __name__ == "__main__":
    main()

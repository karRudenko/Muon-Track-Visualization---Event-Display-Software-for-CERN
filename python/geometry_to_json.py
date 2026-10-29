import json, sys
import yaml 

ANGLE = {"x": 0, "y": 90, "u": 135, "v": 45}   #направление измерения, градусы от оси x

def build(yaml_path):
    cfg = yaml.safe_load(open(yaml_path))
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
            for item in (tc if isinstance(tc, list) else [tc]):   # в yaml это список из пар ключ-значение
                t.update(item)
            target = {"z": origin + st["targetRelativePosition"],
                      "width": t["width"], "height": t["height"], "thickness": t["thickness"]}

        modules = []
        for idx, m in enumerate(st["modules"]):        # idx == ModuleID в ROOT file
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

if __name__ == "__main__":
    json.dump(build(sys.argv[1]), open(sys.argv[2], "w"), indent=2)
    print("written", sys.argv[2])
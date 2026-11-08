#!/usr/bin/env python3
"""Pre-export a set of events to static JSON files 
   For use without the live server.

Examples:
    python3 scripts/export_events.py --root simple_output.root --random 20
    python3 scripts/export_events.py --root simple_output.root --first 50
    python3 scripts/export_events.py --root simple_output.root --events 5 17 42

Writes web/data/events/event_<N>.json and refreshes web/data/events/index.json.
"""
import argparse
import json
import random
import re
from pathlib import Path

from exporter import Exporter, ExporterError

ROOT_DIR = Path(__file__).resolve().parent.parent
DEFAULT_OUT = ROOT_DIR / "web" / "data" / "events"
DEFAULT_BIN = ROOT_DIR / "build" / "bin" / "export_event"


def rebuild_index(out_dir):
    """index.json lists every event_<N>.json that is present in the directory."""
    ids = sorted(int(m.group(1)) for p in out_dir.glob("event_*.json")
                 if (m := re.fullmatch(r"event_(\d+)\.json", p.name)))
    (out_dir / "index.json").write_text(json.dumps({"events": ids}) + "\n")
    return ids


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", required=True, help="ROOT file with the cbmsim tree")
    ap.add_argument("--exporter", default=str(DEFAULT_BIN), help="export_event binary (default: %(default)s)")
    ap.add_argument("--out", default=str(DEFAULT_OUT), help="output directory (default: %(default)s)")
    ap.add_argument("--z-min", type=float, help="start of the detector, cm")
    ap.add_argument("--z-max", type=float, help="end of the detector, cm")
    ap.add_argument("--seed", type=int, help="seed for --random")
    sel = ap.add_mutually_exclusive_group(required=True)
    sel.add_argument("--events", type=int, nargs="+", metavar="N", help="explicit event numbers")
    sel.add_argument("--random", type=int, metavar="K", help="K randomly chosen events")
    sel.add_argument("--first", type=int, metavar="K", help="the first K events")
    args = ap.parse_args()

    z_range = (args.z_min, args.z_max) if args.z_min is not None and args.z_max is not None else None
    try:
        exp = Exporter(args.exporter, args.root, z_range)
        total = exp.count()
        if args.events:
            chosen = args.events
        elif args.random:
            chosen = random.Random(args.seed).sample(range(total), min(args.random, total))
        else:
            chosen = range(min(args.first, total))

        out_dir = Path(args.out)
        out_dir.mkdir(parents=True, exist_ok=True)
        for n in chosen:
            if not 0 <= n < total:
                raise ExporterError(f"event {n} out of range (0..{total - 1})")
            exp.export(n, out_dir / f"event_{n}.json")
            print(f"exported event {n}")
    except ExporterError as exc:
        raise SystemExit(f"error: {exc}")

    ids = rebuild_index(out_dir)
    print(f"{len(ids)} events in {out_dir}")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Development server for the event display.

Serves web/ and, when a ROOT file is given, a small JSON API that calls the C++
`export_event` tool on demand:

    GET /api/info          -> {"nEvents": N, "file": "simple_output.root"}
    GET /api/event/<n>     -> event <n> as JSON (cached on disk)

Without --root it only serves the static files (pre-exported events from web/data/events).

Examples:
    python3 scripts/serve.py --root simple_output.root --config config/simple_3_stations.yaml
    python3 scripts/serve.py    # static mode
"""
import argparse
import hashlib
import json
import os
import re
import threading
from http import HTTPStatus
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

from exporter import Exporter, ExporterError

ROOT_DIR = Path(__file__).resolve().parent.parent
WEB_DIR = ROOT_DIR / "web"
CACHE_DIR = ROOT_DIR / ".cache" / "event-display"
EVENT_RE = re.compile(r"^/api/event/(\d+)$")


class EventBackend:
    """Exports events on demand and caches the resulting JSON files."""

    def __init__(self, exporter: Exporter):
        self.exporter = exporter
        self.n_events = exporter.count()
        stamp = f"{exporter.root_file.resolve()}|{exporter.root_file.stat().st_mtime_ns}|{exporter.z_range}"
        self.cache = CACHE_DIR / hashlib.sha1(stamp.encode()).hexdigest()[:12]
        self.cache.mkdir(parents=True, exist_ok=True)
        self._lock = threading.Lock()

    def info(self):
        return {"nEvents": self.n_events, "file": self.exporter.root_file.name}

    def event_json(self, n):
        path = self.cache / f"event_{n}.json"
        with self._lock:
            if not path.exists():
                tmp = path.with_suffix(".tmp")
                self.exporter.export(n, tmp)
                os.replace(tmp, path)
        return path.read_bytes()


def make_handler(backend):
    class Handler(SimpleHTTPRequestHandler):
        extensions_map = {**SimpleHTTPRequestHandler.extensions_map, ".js": "text/javascript", ".mjs": "text/javascript", ".json": "application/json", ".css": "text/css"}

        def __init__(self, *args, **kwargs):
            super().__init__(*args, directory=str(WEB_DIR), **kwargs)

        def end_headers(self):
            self.send_header("Cache-Control", "no-store")
            super().end_headers()

        def _send(self, status, body, ctype="application/json"):
            self.send_response(status)
            self.send_header("Content-Type", ctype)
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        def _error(self, status, message):
            self._send(status, json.dumps({"error": message}).encode())

        def do_GET(self):
            path = self.path.split("?", 1)[0]
            if not path.startswith("/api/"):
                return super().do_GET()
            if backend is None:
                return self._error(HTTPStatus.NOT_FOUND, "server started without --root (static mode)")
            if path == "/api/info":
                return self._send(HTTPStatus.OK, json.dumps(backend.info()).encode())
            m = EVENT_RE.match(path)
            if not m:
                return self._error(HTTPStatus.NOT_FOUND, "unknown endpoint")
            n = int(m.group(1))
            if n >= backend.n_events:
                return self._error(HTTPStatus.NOT_FOUND, f"event {n} out of range (0..{backend.n_events - 1})")
            try:
                self._send(HTTPStatus.OK, backend.event_json(n))
            except ExporterError as exc:
                self._error(HTTPStatus.INTERNAL_SERVER_ERROR, str(exc))

    return Handler


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", help="ROOT file with the cbmsim tree (enables the live API)")
    ap.add_argument("--exporter", default=str(ROOT_DIR / "build" / "bin" / "export_event"), help="export_event binary (default: %(default)s)")
    ap.add_argument("--config", help="detector YAML; regenerates web/data/geometry.json at startup")
    ap.add_argument("--z-min", type=float, help="start of the detector, cm")
    ap.add_argument("--z-max", type=float, help="end of the detector, cm")
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=8000)
    args = ap.parse_args()

    if args.config:
        from geometry_to_json import write
        print("geometry ->", write(args.config, WEB_DIR / "data" / "geometry.json"))

    backend = None
    if args.root:
        z_range = (args.z_min, args.z_max) if args.z_min is not None and args.z_max is not None else None
        try:
            backend = EventBackend(Exporter(args.exporter, args.root, z_range))
        except ExporterError as exc:
            raise SystemExit(f"error: {exc}")
        print(f"live mode: {backend.n_events} events in {args.root}")
    else:
        print("static mode: serving pre-exported events from web/data/events")

    server = ThreadingHTTPServer((args.host, args.port), make_handler(backend))
    print(f"open http://{args.host}:{args.port}   (Ctrl+C to stop)")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print()


if __name__ == "__main__":
    main()

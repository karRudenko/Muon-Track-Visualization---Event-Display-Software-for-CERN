# MUonE event display

3D web display (three.js) of reconstructed events stored in a ROOT file.

## Layout

```
├── CMakeLists.txt, makefile      build + convenience targets
├── config/                       detector YAML files
├── tools/export_event/           ROOT file -> event JSON (used by the web UI)
│   ├── main.cpp                  command line
│   ├── event_reader.{hpp,cpp}    reads one event from the ROOT tree (the only ROOT-dependent part)
│   ├── json_writer.{hpp,cpp}     writes an event as JSON
│   └── event_data.hpp            plain Hit / Track / Vertex / Event structs
├── scripts/                      Python tooling
│   ├── geometry_to_json.py       detector YAML -> web/data/geometry.json
│   ├── exporter.py               wrapper around the export_event binary
│   ├── export_events.py          pre-export N events to static JSON files
│   └── serve.py                  dev server: static files + live /api/ backed by export_event
└── web/                          the event display (no build step)
    ├── index.html, css/style.css
    ├── js/
    │   ├── main.js               wiring
    │   ├── dataSource.js         live API / static files, EventCatalog (next, prev, random)
    │   ├── scene.js              three.js renderer, camera, layers
    │   ├── detector.js           draws geometry.json
    │   ├── eventView.js          draws hits / tracks / vertices
    │   ├── ui.js                 control panel
    │   └── config.js             colours, paths, defaults
    └── data/                     geometry.json, events/ (static mode)
```

## Data flow

```
 simple_output.root ──► build/bin/export_event ──► event JSON ───┐
                                                                 ├─► browser (web/)
 config/*.yaml ──► scripts/geometry_to_json.py ──► geometry.json ┘
                         ▲
                         │
         scripts/serve.py calls the exporter on demand
```

The browser cannot read ROOT files, so events are converted to JSON first. There are two ways:

* **Live mode** (`make serve`): the server runs `export_event` whenever you pick an event, so *any*
  event of the ROOT file is available. Results are cached in `.cache/event-display/`.
* **Static mode** (`make serve-static`): only the events already exported to `web/data/events/`
  (`make events` exports a random sample). No ROOT needed at runtime, handy for sharing the folder.

The page chooses the mode automatically.

## Quick start

```bash
pip install -r requirements.txt     # pyyaml
make all                            # builds build/bin/export_event (needs ROOT)
make serve ROOT_FILE=simple_output.root
# open http://localhost:8000
```

Other targets: `make events N_EVENTS=50`, `make geometry`, `make serve-static`, `make clean`.

> Open the page via the server, not by double-clicking `index.html`: browsers block `fetch()` on `file://`.
> three.js is loaded from a CDN, so the page needs internet access.

## Using the page

* Type an event number and press Enter / **Go**; leave it empty and press Go for a random event.
* **Random event** (key `R`), **◀ ▶** (keys `←` `→`) step through events.
* `?event=<number>` in the URL opens that event, so links are shareable. Without it a random event is shown.
* **Open a JSON file** shows any file produced by `export_event`.
* Mouse: drag = rotate, wheel = zoom, right-drag = pan. The z-compression slider squeezes the 330 cm long detector.

## Event JSON format

```jsonc
{ "event": 5, "zRange": [550, 880],
  "hits":     [{"station": 0, "module": 0, "z": 566.9, "coord": -1.0, "posInModule": -1.02}],
  "tracks":   [{"p1": [x, y, z], "p2": [x, y, z]}],
  "vertices": [{"pos": [x, y, z], "station": 0, "muonTheta": 0.0, "electronTheta": 0.0}] }
```

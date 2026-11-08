# Usage: "make <target>", e.g. "make all", "make serve"
#
# Build:   all | clean
# Web UI:  serve | serve-static | events | geometry

BUILD       := build
BUILD_TYPE  ?= Release

# --- event display settings (override on the command line, e.g. make serve ROOT_FILE=my.root)
ROOT_FILE   ?= simple_output.root
CONFIG      ?= config/simple_3_stations.yaml
PORT        ?= 8000
# quantity of random events "make events" exports
N_EVENTS    ?= 20
EXPORTER    := $(BUILD)/bin/export_event

all:
	cmake -S . -B $(BUILD) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	cmake --build $(BUILD) -j

# Live event display: the server calls export_event on demand for any event in ROOT_FILE.
serve: all geometry
	python3 scripts/serve.py --root $(ROOT_FILE) --exporter $(EXPORTER) --port $(PORT)

# Static display: only the events already exported to web/data/events (no ROOT needed).
serve-static:
	python3 scripts/serve.py --port $(PORT)

# Pre-export N_EVENTS random events to web/data/events (for static use / sharing).
events: all
	python3 scripts/export_events.py --root $(ROOT_FILE) --exporter $(EXPORTER) --random $(N_EVENTS)

# Regenerate web/data/geometry.json from the detector YAML.
geometry:
	python3 scripts/geometry_to_json.py $(CONFIG) web/data/geometry.json

clean:
	rm -rf $(BUILD)
	rm -rf .cache

.PHONY: all serve serve-static events geometry clean

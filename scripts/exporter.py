"""Thin Python wrapper around the C++ `export_event` tool (ROOT file -> event JSON)."""
import subprocess
from pathlib import Path


class ExporterError(RuntimeError):
    pass


class Exporter:
    def __init__(self, binary, root_file, z_range=None, timeout=120):
        self.binary = Path(binary)
        self.root_file = Path(root_file)
        self.z_range = z_range
        self.timeout = timeout
        if not self.binary.is_file():
            raise ExporterError(f"exporter not found: {self.binary}  (build it first: make all)")
        if not self.root_file.is_file():
            raise ExporterError(f"ROOT file not found: {self.root_file}")

    def _run(self, args):
        cmd = [str(self.binary), str(self.root_file), *map(str, args)]
        try:
            res = subprocess.run(cmd, capture_output=True, text=True, timeout=self.timeout)
        except subprocess.TimeoutExpired as exc:
            raise ExporterError(f"export_event timed out after {self.timeout}s") from exc
        if res.returncode != 0:
            raise ExporterError((res.stderr or res.stdout).strip() or f"export_event failed ({res.returncode})")
        return res.stdout

    def count(self):
        """Number of events in the ROOT file."""
        tokens = self._run(["--count"]).split()
        try:
            return int(tokens[-1])
        except (IndexError, ValueError) as exc:
            raise ExporterError("could not parse event count from export_event") from exc

    def export(self, event, out_path):
        """Write one event to `out_path` as JSON."""
        extra = list(self.z_range) if self.z_range else []
        self._run([event, out_path, *extra])
        return Path(out_path)

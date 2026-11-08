// The control panel: reads the DOM, forwards user actions to callbacks, shows state.

const $ = id => document.getElementById(id);

export function createUI(handlers) {
  const numberInput = $("eventNumber");
  const buttons = ["prev", "next", "go", "random"].map($);
  const info = $("info");

  // ---- event selection ----
  const goto = () => {
    if (numberInput.value === "") return handlers.onRandom();
    handlers.onGoto(Number(numberInput.value));
  };
  $("go").addEventListener("click", goto);
  numberInput.addEventListener("keydown", e => { if (e.key === "Enter") goto(); });
  $("prev").addEventListener("click", () => handlers.onStep(-1));
  $("next").addEventListener("click", () => handlers.onStep(+1));
  $("random").addEventListener("click", () => handlers.onRandom());

  // keyboard shortcuts
  addEventListener("keydown", e => {
    if (e.target.closest?.("input, textarea") || e.ctrlKey || e.metaKey || e.altKey) return;
    if (e.key === "ArrowLeft") handlers.onStep(-1);
    else if (e.key === "ArrowRight") handlers.onStep(+1);
    else if (e.key === "r" || e.key === "R") handlers.onRandom();
  });

  // ---- view options ----
  const zscale = $("zscale");
  const applyZ = () => {
    $("zval").textContent = "x" + (+zscale.value).toFixed(2);
    handlers.onZScale(+zscale.value);
  };
  zscale.addEventListener("input", applyZ);
  document.querySelectorAll("input[data-layer]").forEach(cb =>
    cb.addEventListener("change", () => handlers.onLayerVisible(cb.dataset.layer, cb.checked)));

  return {
    syncZScale: applyZ,

    // Limit the number field to the events that actually exist.
    setCatalog(catalog) {
      const list = $("eventIds");
      list.replaceChildren();
      if (catalog.isRange) {
        numberInput.min = 0;
        numberInput.max = Math.max(0, catalog.size - 1);
        numberInput.placeholder = `0 - ${Math.max(0, catalog.size - 1)}`;
      } else {
        numberInput.removeAttribute("min");
        numberInput.removeAttribute("max");
        numberInput.placeholder = "number";
        for (const id of catalog.ids) list.append(new Option(id));
      }
    },

    setCurrentEvent(id) { numberInput.value = id ?? ""; },
    setBusy(busy) { buttons.forEach(b => { b.disabled = busy; }); document.body.style.cursor = busy ? "progress" : ""; },
    setInfo(text, isError = false) { info.textContent = text; info.className = isError ? "err" : ""; },
  };
}

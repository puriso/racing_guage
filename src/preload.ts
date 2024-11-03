import { contextBridge, ipcRenderer, IpcRendererEvent } from "electron";

interface SensorData {
  temperatureVoltage: number;
  oilPressureVoltage: number;
}

contextBridge.exposeInMainWorld("api", {
  onSensorData: (
    callback: (event: IpcRendererEvent, data: SensorData) => void
  ) => ipcRenderer.on("sensor-data", callback),
});
// preload.ts
console.log("preload.ts loaded");

// renderer.ts
console.log("renderer.ts loaded");
console.log("window.api:", window.api);

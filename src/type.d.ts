import { IpcRendererEvent } from "electron";

// センサーデータの型を定義
interface SensorData {
  temperatureVoltage: number;
  oilPressureVoltage: number;
}

// window.apiの型を定義
interface Api {
  onSensorData: (
    callback: (event: IpcRendererEvent, data: SensorData) => void
  ) => void;
}

// グローバルwindowオブジェクトにapiプロパティを追加
declare global {
  interface Window {
    api: Api;
  }
}

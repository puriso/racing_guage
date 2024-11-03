import { app, BrowserWindow, ipcMain, IpcMainEvent } from "electron";
import { SerialPort } from "serialport";
import { ReadlineParser } from "@serialport/parser-readline";
import * as path from "path";

const isDevelopment = process.env.NODE_ENV === "development";

let mainWindow: BrowserWindow | null;

function createWindow(): void {
  mainWindow = new BrowserWindow({
    width: 800,
    height: 600,
    webPreferences: {
      nodeIntegration: false,
      contextIsolation: true,
      preload: path.join(__dirname, "src/preload.js"),
    },
  });
  mainWindow.loadFile("index.html");
  mainWindow.webContents.openDevTools();

  mainWindow.on("closed", () => {
    mainWindow = null;
  });
}

app.whenReady().then(createWindow);

app.on("window-all-closed", () => {
  if (process.platform !== "darwin") {
    app.quit();
  }
});

app.on("activate", () => {
  if (mainWindow === null) {
    createWindow();
  }
});

// シリアルポート設定
interface Parser {
  on: (event: string, callback: (data: string) => void) => void;
}

let parser: Parser;

console.log("isDevelopment: ", isDevelopment);
if (isDevelopment) {
  console.log("Development mode");
  // ダミーデータを使ったイベントリスナー
  parser = {
    on: (event: string, callback: (data: string) => void) => {
      const mockData = "14,15";
      callback(mockData);
    },
  };
} else {
  const port = new SerialPort({ path: "/dev/ttyUSB0", baudRate: 9600 });
  parser = port.pipe(new ReadlineParser({ delimiter: "\n" })); // Readlineを使用
}

// 各データを格納する変数
let temperatureVoltage = 0;
let oilPressureVoltage = 0;

// シリアルデータ受信時の処理
parser.on("data", (data: string) => {
  const [tempVoltage, oilVoltage] = data.trim().split(",");
  console.log("tempVoltage: ", tempVoltage);
  temperatureVoltage = parseFloat(tempVoltage);
  oilPressureVoltage = parseFloat(oilVoltage);
});

// 一定間隔でデータを送信
setInterval(() => {
  temperatureVoltage = Math.random() * 11;
  if (mainWindow && mainWindow.webContents) {
    mainWindow.webContents.send("sensor-data", {
      temperatureVoltage,
      oilPressureVoltage,
    });
  }
}, 300); // 1000ミリ秒（1秒ごとに更新）

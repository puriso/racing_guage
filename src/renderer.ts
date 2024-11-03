document.addEventListener("DOMContentLoaded", () => {
  if (window.api && window.api.onSensorData) {
    window.api.onSensorData((_event, data) => {
      document.getElementById("tempVoltage")!.innerText =
        data.temperatureVoltage.toFixed(0);
      document.getElementById("oilVoltage")!.innerText =
        data.oilPressureVoltage.toFixed(1);
      console.log("Received sensor data");
    });
  } else {
    console.error("window.api is undefined");
  }
});

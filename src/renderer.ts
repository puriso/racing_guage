document.addEventListener("DOMContentLoaded", () => {
  if (window.api && window.api.onSensorData) {
    window.api.onSensorData((_event, data) => {
      document.getElementById("tempVoltage")!.innerText =
        data.temperatureVoltage.toFixed(2);
      document.getElementById("oilVoltage")!.innerText =
        data.oilPressureVoltage.toFixed(2);
      console.log("Received sensor data");
    });
  } else {
    console.error("window.api is undefined");
  }
});

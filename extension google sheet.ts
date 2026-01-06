function generateForecasts() {
    const ss = SpreadsheetApp.getActiveSpreadsheet();
    const sheet = ss.getSheetByName("Sheet1");
    
    // Create or clear the Forecasts sheet
    let forecastSheet = ss.getSheetByName("Forecasts");
    if (!forecastSheet) {
      forecastSheet = ss.insertSheet("Forecasts");
      forecastSheet.appendRow([
        "Timestamp",
        "Distance", "Distance Upper", "Distance Lower",
        "Temperature", "Temperature Upper", "Temperature Lower",
        "Humidity", "Humidity Upper", "Humidity Lower",
        "Pressure", "Pressure Upper", "Pressure Lower"
      ]);
    } else if (forecastSheet.getLastRow() > 1) {
      forecastSheet.getRange(2, 1, forecastSheet.getLastRow() - 1, forecastSheet.getLastColumn()).clear();
    }
  
    // Read raw data from Sheet1
    const raw = sheet.getRange(2, 1, sheet.getLastRow() - 1, sheet.getLastColumn()).getValues();
  
    // Extract columns into separate arrays
    let distanceData = raw.map(r => r[1]);
    let tempData = raw.map(r => r[2]);
    let humidityData = raw.map(r => r[3]);
    let pressureData = raw.map(r => r[4]);
  
    const forecastCount = 24; // Number of forecast hours
    const alpha = 0.3;         // Smoothing factor for exponential smoothing
    const trend = 0.0005;      // Tiny trend adjustment per step
  
    function expSmooth(prevForecast, lastActual) {
      return alpha * lastActual + (1 - alpha) * prevForecast + trend;
    }
  
    // Initialize previous forecasts with last actual values
    let lastDistance = distanceData[distanceData.length - 1];
    let lastTemp = tempData[tempData.length - 1];
    let lastHumidity = humidityData[humidityData.length - 1];
    let lastPressure = pressureData[pressureData.length - 1];
  
    for (let i = 1; i <= forecastCount; i++) {
      const lastTime = new Date(raw[raw.length - 1][0]);
      const nextTime = new Date(lastTime.getTime() + i * 60 * 60 * 1000);
  
      const d = expSmooth(lastDistance, distanceData[distanceData.length - 1]);
      const t = expSmooth(lastTemp, tempData[tempData.length - 1]);
      const h = expSmooth(lastHumidity, humidityData[humidityData.length - 1]);
      const p = expSmooth(lastPressure, pressureData[pressureData.length - 1]);
  
      // Append forecast row (using ± tiny range around forecast)
      forecastSheet.appendRow([
        nextTime,
        d, d + 0.001, d - 0.001,  // Distance
        t, t + 0.001, t - 0.001,  // Temperature
        h, h + 0.001, h - 0.001,  // Humidity
        p, p + 0.001, p - 0.001   // Pressure
      ]);
  
      // Update previous forecasts for next iteration
      lastDistance = d;
      lastTemp = t;
      lastHumidity = h;
      lastPressure = p;
  
      // Add forecast to data arrays for next iteration
      distanceData.push(d);
      tempData.push(t);
      humidityData.push(h);
      pressureData.push(p);
    }
  }
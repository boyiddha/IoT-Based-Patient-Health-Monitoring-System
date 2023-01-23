// IoT Based Patient Health Monitoring on ESP32 Web Server

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h" 

#define DHTTYPE DHT11 
#define DHTPIN 18
#define DS18B20 5
#define REPORTING_PERIOD_MS     1000
 
float temperature, humidity, BPM, SpO2,bodytemperatureC,bodytemperatureF;
 
/*SSID & Password*/
const char* ssid = "akash";  // Enter SSID here
const char* password = "akash1421";  //Enter Password here
 
DHT dht(DHTPIN, DHTTYPE);; 
PulseOximeter pox;
uint32_t tsLastReport = 0;
OneWire oneWire(DS18B20);
DallasTemperature sensors(&oneWire);
 
 
WebServer server(80); //Object of WebServer(HTTP port, 80 is defult)            
 
void onBeatDetected(){
    Serial.println("Beat!");
}
 
void setup() { // Put your setup code here, to run once
    Serial.begin(9600);
    pinMode(19, OUTPUT);
   // Start the DS18B20 sensor
    sensors.begin();
    sensors.setResolution(12);


    //  Serial.println(F("DHTxx test!"));
    dht.begin();
    delay(500);  
    
    Serial.println("Connecting to ");
    Serial.println(ssid);
   
    //connect to your local wi-fi network
    WiFi.begin(ssid, password);
   
    //check wi-fi is connected to wi-fi network
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected..!");
    Serial.print("Got IP: ");
    Serial.println(WiFi.localIP());
    delay(2000);
   
    server.on("/", handle_OnConnect);
    server.onNotFound(handle_NotFound);
   
    server.begin();
    Serial.println("HTTP server started");
   
    Serial.print("Initializing pulse oximeter....");
    if (!pox.begin()) {
        Serial.println("FAILED");
        for (;;);
    }
    else {
        Serial.println("SUCCESS");
        pox.setOnBeatDetectedCallback(onBeatDetected);
    }
   
     pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);//setIRLedCurrent() function sets the current through the IR LED. The following line sets the current of 7.6mA through the IR LED.
   
    // Register a callback for the beat detection
    pox.setOnBeatDetectedCallback(onBeatDetected);
 
}
void loop() { // Put your main code here, to run repeatedly.....
    server.handleClient();

        // Make sure to call update as fast as possible
    pox.update();
    BPM = pox.getHeartRate();
    SpO2 = pox.getSpO2();
    
    float t = dht.readTemperature();
    String Temperature_Value = String(t);
    float h = dht.readHumidity();
    String Humidity_Value = String(h);
      // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);
      // Check if any reads failed and exit early (to try again).
    temperature=t;
    humidity=h;
    if (isnan(h) || isnan(t)||isnan(f)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
    }
  
    bodytemperatureC = sensors.getTempCByIndex(0);
    bodytemperatureF = sensors.getTempFByIndex(0);

    sensors.setWaitForConversion(false);            // requestTemperatures() will not block current thread
    sensors.requestTemperatures(); 
    sensors.setWaitForConversion(true);    
   
    
    if ( millis() - tsLastReport > REPORTING_PERIOD_MS ){
        Serial.print("Room Temperature: ");
        Serial.print(t);
        Serial.println("°C");
    
        Serial.print("Room Humidity: ");
        Serial.print(h);
        Serial.println("%");
    
        Serial.print("BPM: ");
        Serial.println(BPM);
    
        Serial.print("SpO2: ");
        Serial.print(SpO2);
        Serial.println("%");
    
        Serial.print("Body Temperature: ");
        Serial.print(bodytemperatureC);
        Serial.println("ºC");
        Serial.print(bodytemperatureF);
        Serial.println("ºF");
    
        Serial.println("*********************************");
        Serial.println();
     
        tsLastReport = millis();
    }
  
}
 
void handle_OnConnect() {
    server.send(200, "text/html", SendHTML(temperature, humidity, BPM, SpO2, bodytemperatureC,bodytemperatureF)); 
}
 
void handle_NotFound(){
    server.send(404, "text/plain", "Not found");
}

String SendHTML( float temperature, float humidity, float BPM, float SpO2, float bodytemperatureC,float bodytemperatureF) {
    String html = "<!DOCTYPE html>";
    html += "<html>";
    html += "<head>";
    html += "<title>Patient Health Monitoring</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
            // font awesome cdn
    html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.7.2/css/all.min.css'>";
             //google fonts
    html += "<link href='https://fonts.googleapis.com/css2?family=Londrina+Outline&display=swap' rel='stylesheet'>";
    
    html += "<style>";
    html += "body { center; background-color: #1fa622; font-family: sans-serif; color: #333333; font: 14px Helvetica, sans-serif box-sizing: border-box;}";
    html += "#page { margin: 20px; background-color: #670c75;}";
    html += ".header { padding: 5px;}";
    html += ".header h1 { padding-bottom: 0.3em; color:#1ad9a6; font-size: 45px; font-weight: bold; font-family: 'Londrina Outline', cursive; text-align: center;}";
    
    html += ".datetime{color: #fff;background: #10101E;font-family: 'Segoe UI', sans-serif;width: 320px;height:60px; margin:auto;padding: 15px 10px; border: 3px solid #2E94E3; border-radius: 5px;-webkit-box-reflect: below 1px linear-gradient(transparent, rgba(255, 255, 255, 0.1)); transition: 0.5s;transition-property: background, box-shadow;}";
    html += ".datetime:hover{ background: #2E94E3; box-shadow: 0 0 30px #2E94E3;}";
    html += ".date{font-size: 15px;font-weight: 500;text-align: center;letter-spacing: 3px;}";
    html += ".time{font-size: 40px;display: flex;justify-content: center;align-items: center;}";
    html += ".time span:not(:last-child){position: relative;margin: 0 6px;font-weight: 500;text-align: center;letter-spacing: 3px;}";
    html += ".time span:last-child{ background: #2E94E3; font-size: 20px; font-weight: 500; text-transform: uppercase;margin-top: 10px;padding: 0 5px;border-radius: 3px;}";
    
    html += "h2 { padding-bottom: 0.2em; border-bottom: 1px solid #eee; margin: 2px; text-align: left;color:red;}";
    html += ".box-full { padding: 20px; border 1px solid #ddd; border-radius: 1em 1em 1em 1em; box-shadow: 1px 7px 7px 1px rgba(0,0,0,0.4); background: #fff; margin: 20px; width: 300px;}";
    html += "@media (max-width: 494px) { #page { width: inherit; margin: 5px auto; } #content { padding: 1px;} .box-full { margin: 8px 8px 12px 8px; padding: 10px; width: inherit;; float: none; } }";
    html += "@media (min-width: 494px) and (max-width: 980px) { #page { width: 465px; margin 0 auto; } .box-full { width: 380px; } }";
    html += "@media (min-width: 980px) { #page { width: 930px; margin: auto; } }";
    
    html += ".sensor { margin: 12px 0px; font-size: 2.5rem;}";
    html += ".sensor:hover{background: #05fff7;box-shadow: 0 0 30px #2E94E3;}";
    html += ".sensor-labels { font-size: 1rem; vertical-align: middle; padding-bottom: 15px;}";
    html += ".units { font-size: 1.2rem;}";
    html += "hr{ height: 2px; color: green; background-color:green; border: normal;}";
    html += ".footer{position: fixed; left: 0; bottom: 0px; height:55px; width: 100%; background-color: gray; color: white;text-align: center;}";
    
    html += "</style>";
    
            //Ajax Code Start
    html += "<script>\n";
    html += "setInterval(loadDoc,1000);\n";
    html += "function loadDoc() {\n";
    html += "var xhttp = new XMLHttpRequest();\n";
    html += "xhttp.onreadystatechange = function() {\n";
    html += "if (this.readyState == 4 && this.status == 200) {\n";
    html += "document.body.innerHTML =this.responseText}\n";
    html += "};\n";
    html += "xhttp.open(\"GET\", \"/\", true);\n";
    html += "xhttp.send();\n";
    html += "}\n";
    html += "</script>\n";
            //Ajax Code END
    
            //start Javascript logic for digital clock
    html += "<script type='text/javascript'>\n";
    html += "function updateClock(){\n";
    html += "var now = new Date();\n";
    html += "var dname = now.getDay(),mo = now.getMonth(),dnum = now.getDate(),yr = now.getFullYear(),hou = now.getHours(),min = now.getMinutes(),sec = now.getSeconds(),pe = 'AM';\n";

    html += "if(hou >= 12){pe = 'PM';}\n";
    html += "if(hou == 0){hou = 12;}\n";
    html += "if(hou > 12){hou = hou - 12;}\n";

    html += "Number.prototype.pad = function(digits){\n";
    html += "for(var n = this.toString(); n.length < digits; n = 0 + n);\n";
    html += "return n;\n";
    html += "}\n";

    html += "var months = ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December'];\n";
    html += "var week = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'];\n";
    html += "var ids = ['dayname', 'month', 'daynum', 'year', 'hour', 'minutes', 'seconds', 'period'];\n";
    html += "var values = [week[dname], months[mo], dnum.pad(2), yr, hou.pad(2), min.pad(2), sec.pad(2), pe];\n";
    html += "for(var i = 0; i < ids.length; i++)\n";
    html += "document.getElementById(ids[i]).firstChild.nodeValue = values[i];\n";
    html += "}\n";

    html += "function initClock(){\n";
    html += "updateClock();\n";
    html += "window.setInterval('updateClock()', 1);\n";
    html += "}\n";
    html += "</script>\n";
           // end Javascript logic for digital clock 
         
    html += "</head>";
    
    html += "<body onload='initClock()'>";
    html += "<div id='page'>";
    html += "<div class='header'>";
    html += "<h1>Health Monitoring System</h1>";
    html += "</div>";
    
             //digital clock start
    html += "<div class='datetime'>";
    html += "<div class='date'>";
    html += "<span id='dayname'>Day</span>,";
    html += "<span id='month'>Month</span>";
    html += "<span id='daynum'>00</span>,";
    html += "<span id='year'>Year</span>";
    html += "</div>";
    html += "<div class='time'>";
    html += "<span id='hour'>00</span>:";
    html += "<span id='minutes'>00</span>:";
    html += "<span id='seconds'>00</span>";
    html += "<span id='period'>AM</span>";
    html += "</div>";
    html += "</div>";
            //digital clock end-
          
    html += "<div id='content' align='center'>";
    html += "<div class='box-full' align='left'>";
    html += "<h2>&emsp;&emsp;Sensors Readings</h2>";
    html += "<div class='sensors-container'>";
    
             //For Temperature
    html += "<div class='sensors'>";
    html += "<p class='sensor'>";
    html += "<i class='fas fa-thermometer-half' style='color:#0275d8'></i>";
    html += "<span class='sensor-labels'> Room Temperature </span>";
    html += (int)temperature;
    html += "<sup class='units'>°C</sup>";
    html += "</p>";
    html += "<hr>";
    html += "</div>";
    
             //For Humidity
    html += "<div class='sensors'>";
    html += "<p class='sensor'>";
    html += "<i class='fas fa-tint' style='color:#5bc0de'></i>";
    html += "<span class='sensor-labels'> Room Humidity </span>";
    html += (int)humidity;
    html += "<sup class='units'>%</sup>";
    html += "</p>";
    html += "<hr>";
    
             //For Heart Rate
    html += "<p class='sensor'>";
    html += "<i class='fas fa-heartbeat' style='color:#cc3300'></i>";
    html += "<span class='sensor-labels'> Heart Rate </span>";
    html += (int)BPM;
    html += "<sup class='units'>BPM</sup>";
    html += "</p>";
    html += "<hr>";
    
             //For Sp02
    html += "<p class='sensor'>";
    html += "<i class='fas fa-burn' style='color:#f7347a'></i>";
    html += "<span class='sensor-labels'> Sp02 </span>";
    html += (int)SpO2;
    html += "<sup class='units'>%</sup>";
    html += "</p>";
    html += "<hr>";
    
             //For Body Temperature celsius
    html += "<p class='sensor'>";
    html += "<i class='fas fa-thermometer-full' style='color:#d9534f'></i>";
    html += "<span class='sensor-labels'> Body Temperature </span>";
    html += (int)bodytemperatureC;
    html += "<sup class='units'>°C</sup>";
    html += "</p>";
    html += "<hr>";
    
             //For Body Temperature in fahrenheit
    html += "<p class='sensor'>";
    html += "<i class='fas fa-thermometer-full' style='color:#d9534f'></i>";
    html += "<span class='sensor-labels'> Body Temperature </span>";
    html += (int)bodytemperatureF;
    html += "<sup class='units'>°F</sup>";
    html += "</p>";
    html += "<hr><hr><br>";
    
    html += "</div>";
    html += "</div>";
    html += "</div>";
    html += "</div>";
    html += "</div>";
    
    html += "<div class='footer'>";
    html += " <br>Copyright &copy; boyiddha 2022.<br> Developed by Boyiddhanath Roy.<br>.";
    html += "</div>";
        
    html += "</body>";
    html += "</html>";
    
    return html;
}

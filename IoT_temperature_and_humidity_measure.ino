#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <DHT.h> 
#include <DHT_U.h>

#define SCREEN_WIDTH 128 // OLED display width,  in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
// declare an SSD1306 display object connected to I2C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 
String apiKey = "XXXXXXXXXXXXXXX";     //  Enter your Write API key from ThingSpeak
 
const char *ssid =  "XXXXXXXXXXXXXXXXX";     // replace with your wifi ssid and wpa2 key
const char *pass =  "XXXXXXXXXXXXXXXXX";
const char* server = "api.thingspeak.com";
 
#define DHTPIN 13          //pin where the dht11 is connected

#define DHTTYPE    DHT22     // DHT 22 (AM2302)

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;
 
WiFiClient client;
 
void setup() 
{
       Serial.begin(115200);
       //Connect to WiFi
       delay(10);
       dht.begin();
 
       Serial.println("Connecting to ");
       Serial.println(ssid);
 
 
       WiFi.begin(ssid, pass);
 
      while (WiFi.status() != WL_CONNECTED) 
     {
            delay(500);
            Serial.print(".");
     }
      Serial.println("");
      Serial.println("WiFi connected");

      // initialize OLED display with address 0x3C for 128x32
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  delay(2000);         // wait for initializing
  display.clearDisplay(); // clear display

  display.setTextSize(1);          // text size
  display.setTextColor(WHITE);     // text color
  display.setCursor(15, 10);        // position to display
  display.println("SYSTEM INITIALIZE"); // text to display
  display.display();               // show on OLED

  // Initialize device.
  dht.begin();
  Serial.println(F("DHTxx Unified Sensor Example"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;
}
 
void loop() 
{
  // Delay between measurements.
  delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
  }

  //Send to thingspeak 
  if (client.connect(server,80))   //   "184.106.153.149" or api.thingspeak.com
  {  
    //Send temperature value
    String postStr = apiKey;
    postStr +="&field1=";
    postStr += String(event.temperature);
  
  // Display output for temperature
  display.clearDisplay();
  display.setTextSize(1);          // text size
  display.setTextColor(WHITE);     // text color
  display.setCursor(0, 0);        // position to display
  display.println("IoT Temp & RH sensor"); // text to display
  display.display();              // show on OLED
  display.setCursor(0, 10);        // position to display
  display.println("Temp"); // text to display
  display.display();               // show on OLED
 
  display.setTextSize(1);
  display.setCursor(30, 10); 
  display.print(event.temperature);
  display.display();
  display.setCursor(60, 10); 
  display.println(" `C");
  display.display();

  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
  }
    //Send relative humidity
    postStr +="&field2=";
    postStr += String(event.relative_humidity);
    postStr += "\r\n\r\n";

  // Display output for humidity
  display.setTextSize(1);          // text size
  display.setTextColor(WHITE);     // text color
  display.setCursor(0, 20);        // position to display
  display.print("RH"); // text to display
  display.display();               // show on OLED
 
  display.setTextSize(1);
  display.setCursor(30, 20); 
  display.print(event.relative_humidity);
  display.display();
  display.setCursor(60, 20); 
  display.println(" %");
  display.display();

  //Client protocol
  client.print("POST /update HTTP/1.1\n");
  client.print("Host: api.thingspeak.com\n");
  client.print("Connection: close\n");
  client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
  client.print("Content-Type: application/x-www-form-urlencoded\n");
  client.print("Content-Length: ");
  client.print(postStr.length());
  client.print("\n\n");
  client.print(postStr);
 
  Serial.print("Temperature: ");
  Serial.print(event.temperature);
  Serial.print(" degrees Celcius, Humidity: ");
  Serial.print(event.relative_humidity);
  Serial.println("%. Send to Thingspeak.");
  }

  client.stop();
  Serial.println("Waiting...");
  
  // thingspeak needs minimum 15 sec delay between updates
  delay(1000);
}

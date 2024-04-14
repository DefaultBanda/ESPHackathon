#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Replace with your network credentials and API key
const char* ssid = "Hello";
const char* password = "Hmmm";
const char* apiKey = "BeepBOP";
const char* city = "Toronto"; // Replace with your city

// Timezone offset for EST (UTC-5:00)
// If you need to handle daylight saving time, adjust this value accordingly
const long utcOffsetInSeconds = -14400; 

// Display pins
#define TFT_CS   17
#define TFT_RST  5  // C
#define TFT_DC   16
#define TFT_MOSI 23 // SDA
#define TFT_CLK  18 // SCK
#define TFT_MISO 19 // SDO
#define TFT_LED  32 // 

// SD and Touch pins 
// #define SD_CS   12
// #define TOUCH_CS 21

// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// Initialize the ILI9341
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

void setup() {
    Serial.begin(115200);

    // Initialize WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
    
    // Initialize the display
    tft.begin();
    tft.setRotation(3); n
    tft.fillScreen(ILI9341_BLACK);

    // Start the NTP client
    timeClient.begin();
}

void loop() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 60000) {
        lastUpdate = millis();
        updateTime();
        updateWeather();
    }
}

void updateTime() {
    timeClient.update();
    String formattedTime = timeClient.getFormattedTime().substring(0, 5); // Get HH:MM format
    tft.fillRect(0, 0, 240, 40, ILI9341_BLACK); // Clear the area for time display
    tft.setCursor(10, 10); // Adjust as needed
    tft.setTextColor(ILI9341_WHITE);  
    tft.setTextSize(2);
    tft.println(formattedTime);
}

void updateWeather() {
    if (WiFi.status() == WL_CONNECTED) {
        String weatherQueryUrl = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "&appid=" + apiKey + "&units=metric";
        HTTPClient http;
        http.begin(weatherQueryUrl.c_str());
        int httpCode = http.GET();
        if (httpCode > 0) {
            String payload = http.getString();
            Serial.println("Weather Data received successfully.");
            displayWeather(payload);
        } else {
            Serial.println("Error fetching weather data: " + String(httpCode));
        }
        http.end();
    } else {
        Serial.println("WiFi Disconnected. Cannot fetch weather data.");
    }
}

void displayWeather(const String& json) {
    StaticJsonDocument<1024> doc;
    deserializeJson(doc, json);
    float temperature = doc["main"]["temp"];
    String conditions = doc["weather"][0]["main"].as<String>();
    
    tft.fillRect(0, 40, 240, 80, ILI9341_BLACK); 
    tft.setCursor(10, 50); 
    tft.setTextColor(ILI9341_WHITE);  
    tft.setTextSize(2);
    tft.print("Temp: ");
    tft.print(temperature);
    tft.println(" C");

    tft.setCursor(10, 70); 
    tft.print("Cond: ");
    tft.println(conditions);
}

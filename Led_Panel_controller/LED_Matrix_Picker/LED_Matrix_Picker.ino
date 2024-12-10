#include <FastLED.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// Define the LED strip and matrix size
#define MAX_LEDS 512  // Reduced the maximum number of LEDs to 512
#define DATA_PIN 5    // GPIO pin connected to the LED strip

CRGB leds[MAX_LEDS];  // LED array

// Wi-Fi credentials
const char* ssid = "iPhone";         // Your Wi-Fi SSID
const char* password = "doyourbest."; // Your Wi-Fi password

// Create an AsyncWebServer object on port 80
AsyncWebServer server(80);

// Matrix dimensions
int matrixWidth = 4;  // Default matrix width (4)
int matrixHeight = 4; // Default matrix height (4)

// HTML for the main page
String generateHtml() {
  String html = "<html><head><style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; padding: 20px; }";
  html += "h1 { color: #333; }";
  html += "form { display: inline-block; margin: 5px; }";
  html += "input[type='submit'] { padding: 10px 15px; font-size: 14px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; transition: background-color 0.3s; }";
  html += "input[type='submit']:hover { background-color: #45a049; }";
  
  // Container for the LED matrix (100mm x 100mm max)
  html += "#ledMatrix { display: grid; grid-template-columns: repeat(" + String(matrixWidth) + ", 1fr); grid-gap: 5px; justify-items: center; align-items: center; width: 100mm; height: 100mm; border: 2px solid #333; margin: 20px auto; padding: 10px; box-sizing: border-box; }";
  
  // Ensure button size adjusts based on grid size
  html += "#ledMatrix input[type='submit'] { width: calc(100% - 10px); height: calc(100% - 10px); font-size: 12px; }";
  html += "</style></head><body>";
  html += "<h1>LED Matrix Controller</h1>";

  // Matrix size input form
  html += "<form action=\"/setMatrix\" method=\"GET\">";
  html += "Width: <input type=\"number\" name=\"width\" value=\"" + String(matrixWidth) + "\" min=\"1\" max=\"300\"><br>";
  html += "Height: <input type=\"number\" name=\"height\" value=\"" + String(matrixHeight) + "\" min=\"1\" max=\"300\"><br>";
  html += "<input type=\"submit\" value=\"Set Matrix\">";
  html += "</form><br>";

  // Create buttons for each LED in the matrix, no labels
  html += "<div id=\"ledMatrix\">";
  for (int row = 0; row < matrixHeight; row++) {
    for (int col = 0; col < matrixWidth; col++) {
      int ledIndex = row * matrixWidth + col;
      html += "<form action=\"/toggleLED\" method=\"GET\">";
      html += "<input type=\"hidden\" name=\"ledIndex\" value=\"" + String(ledIndex) + "\">";
      html += "<input type=\"submit\" value=\"\"> ";
      html += "</form>";
    }
  }
  html += "</div>";

  html += "</body></html>";
  return html;
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set up FastLED
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, MAX_LEDS);
  FastLED.setBrightness(100);
  FastLED.clear();  // Turn off all LEDs initially
  FastLED.show();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Serve the main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = generateHtml();
    request->send(200, "text/html", html);
  });

  // Handle matrix size setting
  server.on("/setMatrix", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("width") && request->hasParam("height")) {
      matrixWidth = request->getParam("width")->value().toInt();
      matrixHeight = request->getParam("height")->value().toInt();
      
      // Ensure that the number of LEDs does not exceed the max limit (512 LEDs)
      if (matrixWidth * matrixHeight > MAX_LEDS) {
        matrixWidth = 512;  // If it exceeds, set max width and height to match 512 LEDs
        matrixHeight = 1;   // Adjust to fit the max limit (1 row and 512 columns for example)
      }
    }
    String html = generateHtml();
    request->send(200, "text/html", html);
  });

  // Handle LED toggling
  server.on("/toggleLED", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("ledIndex")) {
      int ledIndex = request->getParam("ledIndex")->value().toInt();
      if (ledIndex >= 0 && ledIndex < MAX_LEDS) {
        // Toggle LED state between red and off
        if (leds[ledIndex] == CRGB::Red) {
          leds[ledIndex] = CRGB::Black;  // Turn off LED
        } else {
          leds[ledIndex] = CRGB::Red;    // Turn on LED (Red)
        }
        FastLED.show();
      }
    }
    String html = generateHtml();  // Reload the page with the updated state
    request->send(200, "text/html", html);
  });

  // Start the server
  server.begin();
}

void loop() {
  // Nothing needed here for now, server handles everything
}

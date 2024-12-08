// Include necessary libraries
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <FastLED.h>

// Wi-Fi credentials
const char* ssid = "iPhone";
const char* password = "doyourbest.";

// LED configuration
#define LED_PIN 5             // Pin connected to the WS2812B data line
#define NUM_LEDS 30           // Total number of LEDs in the strip
CRGB leds[NUM_LEDS];          // Array to hold LED colors

// Web server on port 80
AsyncWebServer server(80);

// Function to set LED color
void setColor(String color) {
  if (color.length() != 7 || color[0] != '#') {
    Serial.println("Invalid color format: " + color);
    return;
  }

  uint32_t hexColor = strtol(color.substring(1).c_str(), NULL, 16); // Convert HEX color to integer
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB((hexColor >> 16) & 0xFF, (hexColor >> 8) & 0xFF, hexColor & 0xFF);
  }
  FastLED.show();
  Serial.println("Color changed to: " + color);
}

// HTML/CSS/JavaScript for radial color picker as a raw string literal
const char webpage[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
    <head>
        <meta charset="utf-8" />
        <title>Radial Color Picker</title>
        <meta http-equiv="X-UA-Compatible" content="IE=Edge" />
        <meta name="viewport" content="width=device-width, initial-scale=1" />
        <script src="https://unpkg.com/vue@3.4.21/dist/vue.global.prod.js"></script>
        <script src="https://unpkg.com/@radial-color-picker/vue-color-picker/dist/vue-color-picker.umd.min.js"></script>
        <link rel="stylesheet" href="https://unpkg.com/@radial-color-picker/vue-color-picker/dist/vue-color-picker.min.css" />
        <style>
            #app {
                font-family: 'Avenir', Helvetica, Arial, sans-serif;
                -webkit-font-smoothing: antialiased;
                -moz-osx-font-smoothing: grayscale;
                display: flex;
                flex-direction: column;
                align-items: center;
                color: #2c3e50;
                margin-top: 40px;
            }
            h1 {
                font-weight: normal;
            }
            pre {
                min-width: 275px;
                padding: 15px 30px;
                background: #f8f8f8;
                color: #525252;
                font-size: 15px;
                font-weight: bold;
                line-height: 1.6;
                margin: 0;
            }
            @media (max-width: 420px) {
                h1 {
                    font-size: 1.4em;
                }
            }
        </style>
    </head>
    <body>
        <div id="app">
            <h1>Pick a Color</h1>
            <color-picker v-bind="color" @input="onInput"></color-picker>
            <pre>{{ color }}</pre>
        </div>

        <script>
            var ColorPicker = window.VueColorPicker;

            Vue.createApp({
                components: {
                    ColorPicker: ColorPicker,
                },
                data() {
                    return {
                        color: {
                            hue: 50,
                            saturation: 100,
                            luminosity: 50,
                            alpha: 1,
                        },
                    };
                },
                methods: {
                    onInput: function (hue) {
                        this.color.hue = hue;

                        // Convert HSL to RGB and send to ESP32/ESP8266
                        const hsl = `hsl(${hue}, ${this.color.saturation}%, ${this.color.luminosity}%)`;
                        const ctx = document.createElement('canvas').getContext('2d');
                        ctx.fillStyle = hsl;
                        const hexColor = ctx.fillStyle;
                        
                        console.log('Selected Color (HEX): ' + hexColor);
                        
                        // Send color to server
                        var xhttp = new XMLHttpRequest();
                        xhttp.open('GET', '/setColor?color=' + encodeURIComponent(hexColor), true);
                        xhttp.send();
                    },
                },
            }).mount('#app');
        </script>
    </body>
</html>
)rawliteral";

void setup() {
  // Start serial communication
  Serial.begin(115200);
  
  // Set up the LED strip
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  // Connect to Wi-Fi in STA + AP mode
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  WiFi.softAP("ESP_Color_Control", "12345678"); // AP mode
  
  // Wait for STA mode connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConnected to WiFi!");
  Serial.println("IP Address (STA): " + WiFi.localIP().toString());
  Serial.println("IP Address (AP): " + WiFi.softAPIP().toString());

  // Serve the HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", webpage);
  });

  // Handle color change requests
  server.on("/setColor", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("color")) {
      String color = request->getParam("color")->value();
      if (color.length() > 0) {
        setColor(color);
      } else {
        Serial.println("Error: color parameter is empty");
      }
    } else {
      Serial.println("Error: color parameter not found");
    }
    request->send(200, "text/plain", "OK");
  });

  // Start the server
  server.begin();
}

void loop() {
  // No need for code in loop since the server is async
}

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
uint8_t brightness = 128;     // Default brightness (0-255)

// Web server on port 80
AsyncWebServer server(80);

// Function to set LED color instantly
void setColor(String color) {
  if (color.length() != 7 || color[0] != '#') {
    Serial.println("Invalid color format: " + color);
    return;
  }

  uint32_t hexColor = strtol(color.substring(1).c_str(), NULL, 16); // Convert HEX color to integer
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB((hexColor >> 16) & 0xFF, (hexColor >> 8) & 0xFF, hexColor & 0xFF);
  }
  FastLED.setBrightness(brightness);
  FastLED.show(); // Immediate update
  Serial.println("Color changed to: " + color);
}

// Function to set LED brightness instantly
void setBrightness(int newBrightness) {
  brightness = constrain(newBrightness, 0, 255);
  FastLED.setBrightness(brightness);
  FastLED.show(); // Immediate update
  Serial.println("Brightness changed to: " + String(brightness));
}

// HTML/CSS/JavaScript for radial color picker, color buttons, and brightness slider
const char webpage[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
    <head>
        <meta charset="utf-8" />
        <title>LED Control</title>
        <meta name="viewport" content="width=device-width, initial-scale=1" />
        <script src="https://unpkg.com/vue@3.4.21/dist/vue.global.prod.js"></script>
        <script src="https://unpkg.com/@radial-color-picker/vue-color-picker/dist/vue-color-picker.umd.min.js"></script>
        <link rel="stylesheet" href="https://unpkg.com/@radial-color-picker/vue-color-picker/dist/vue-color-picker.min.css" />
        <style>
            #app {
                font-family: 'Avenir', Helvetica, Arial, sans-serif;
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
            .color-buttons {
                display: flex;
                gap: 10px;
                margin: 20px 0;
            }
            .color-button {
                width: 40px;
                height: 40px;
                border-radius: 50%;
                border: none;
                cursor: pointer;
            }
            .brightness-slider {
                margin-top: 20px;
                width: 80%;
            }
        </style>
    </head>
    <body>
        <div id="app">
            <h1>Pick a Color</h1>

            <div class="color-buttons">
                <button class="color-button" v-for="color in defaultColors" 
                        :style="{ backgroundColor: color }" 
                        @click="setColor(color)">
                </button>
            </div>

            <color-picker v-bind="color" @input="onInput"></color-picker>
            <pre>{{ color }}</pre>

            <h1>Brightness</h1>
            <input type="range" min="0" max="255" v-model="brightness" @input="setBrightness" class="brightness-slider">
            <pre>Brightness: {{ brightness }}</pre>
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
                        brightness: 128,
                        defaultColors: ['#FF0000', '#00FF00', '#0000FF', '#FFFF00', '#FF00FF', '#00FFFF', '#FFA500', '#800080']
                    };
                },
                methods: {
                    onInput: function (hue) {
                        const ctx = document.createElement('canvas').getContext('2d');
                        ctx.fillStyle = `hsl(${hue}, ${this.color.saturation}%, ${this.color.luminosity}%)`;
                        const hexColor = ctx.fillStyle;
                        this.sendColorToServer(hexColor);
                    },
                    setColor: function (color) {
                        this.sendColorToServer(color);
                    },
                    setBrightness: function () {
                        var xhttp = new XMLHttpRequest();
                        xhttp.open('GET', '/setBrightness?value=' + this.brightness, true);
                        xhttp.send();
                    },
                    sendColorToServer: function (color) {
                        var xhttp = new XMLHttpRequest();
                        xhttp.open('GET', '/setColor?color=' + encodeURIComponent(color), true);
                        xhttp.send();
                    }
                },
            }).mount('#app');
        </script>
    </body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  WiFi.softAP("ESP_Color_Control", "12345678");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi!");
  Serial.println("IP Address: " + WiFi.localIP().toString());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", webpage);
  });

  server.on("/setColor", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("color")) {
      String color = request->getParam("color")->value();
      setColor(color);
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/setBrightness", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("value")) {
      int brightness = request->getParam("value")->value().toInt();
      setBrightness(brightness);
    }
    request->send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  // Nothing needed in loop
}

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>

// WiFi credentials
const char *ssid = "iPhone";
const char *password = "doyourbest.";

// LED configuration
#define LED_PIN 5             // Pin connected to the WS2812B data line
#define NUM_LEDS 30           // Total number of LEDs in the strip
CRGB leds[NUM_LEDS];          // Array to hold LED colors
uint8_t brightness = 128;     // Default brightness (0-255)

// Create AsyncWebServer
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

// HTML content for the main page
const char colorPickerPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>LED Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <script src="https://unpkg.com/vue@3.4.21/dist/vue.global.prod.js"></script>
    <script src="https://unpkg.com/@radial-color-picker/vue-color-picker/dist/vue-color-picker.umd.min.js"></script>
    <link rel="stylesheet" href="https://unpkg.com/@radial-color-picker/vue-color-picker/dist/vue-color-picker.min.css">
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
            background: #e0e5ec;
            color: #333;
        }

        h1 {
            font-weight: 600;
            margin-bottom: 20px;
            color: #555;
        }

        .neumorphic {
            background: #e0e5ec;
            box-shadow: 9px 9px 16px #b8bcc2, -9px -9px 16px #ffffff;
            border-radius: 12px;
            border: none;
            outline: none;
        }

        .color-button {
            width: 50px;
            height: 50px;
            margin: 10px;
            border-radius: 50%;
            border: none;
            cursor: pointer;
            transition: box-shadow 0.2s;
        }

        .color-button:hover {
            box-shadow: 2px 2px 5px rgba(0, 0, 0, 0.2);
        }

        input[type="range"] {
            -webkit-appearance: none;
            width: 80%;
            margin: 20px 0;
        }

        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            background: #e0e5ec;
            border-radius: 50%;
            width: 20px;
            height: 20px;
            box-shadow: 5px 5px 10px #b8bcc2, -5px -5px 10px #ffffff;
        }

        .nav-bar {
            position: fixed;
            bottom: 0;
            width: 100%;
            display: flex;
            justify-content: space-evenly;
            background: #e0e5ec;
            padding: 10px 0;
            box-shadow: 0 -2px 5px rgba(0, 0, 0, 0.1);
            border-radius: 12px 12px 0 0;
        }

        .nav-item {
            flex: 1;
            text-align: center;
            color: #555;
            padding: 10px 0;
            transition: color 0.2s, box-shadow 0.2s;
            border-radius: 8px;
        }

        .nav-item:hover {
            color: #ff5722;
            box-shadow: 2px 2px 5px rgba(0, 0, 0, 0.2);
        }

        .nav-item.active {
            color: #ff5722;
            font-weight: bold;
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

    <div class="nav-bar">
        <div class="nav-item" onclick="window.location.href='/home'">Home</div>
        <div class="nav-item active" onclick="window.location.href='/explore'">Explore</div>
        <div class="nav-item" onclick="window.location.href='/search'">Search</div>
        <div class="nav-item" onclick="window.location.href='/library'">Library</div>
    </div>
    <script>
        const ColorPicker = window.VueColorPicker;

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
                onInput(hue) {
                    const ctx = document.createElement('canvas').getContext('2d');
                    ctx.fillStyle = `hsl(${hue}, ${this.color.saturation}%, ${this.color.luminosity}%)`;
                    const hexColor = ctx.fillStyle;
                    this.sendColorToServer(hexColor);
                },
                setColor(color) {
                    this.sendColorToServer(color);
                },
                setBrightness() {
                    fetch(`/setBrightness?value=${this.brightness}`);
                },
                sendColorToServer(color) {
                    fetch(`/setColor?color=${encodeURIComponent(color)}`);
                }
            },
        }).mount('#app');
    </script>
</body>
</html>
)rawliteral";


const char explorePage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>LED Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
            background: #e0e5ec;
            color: #333;
        }

        h1 {
            font-weight: 600;
            margin-bottom: 20px;
            color: #555;
        }

        .neumorphic {
            background: #e0e5ec;
            box-shadow: 9px 9px 16px #b8bcc2, -9px -9px 16px #ffffff;
            border-radius: 12px;
            border: none;
            outline: none;
        }
        .nav-bar {
            position: fixed;
            bottom: 0;
            width: 100%;
            display: flex;
            justify-content: space-evenly;
            background: #e0e5ec;
            padding: 10px 0;
            box-shadow: 0 -2px 5px rgba(0, 0, 0, 0.1);
            border-radius: 12px 12px 0 0;
        }

        .nav-item {
            flex: 1;
            text-align: center;
            color: #555;
            padding: 10px 0;
            transition: color 0.2s, box-shadow 0.2s;
            border-radius: 8px;
        }

        .nav-item:hover {
            color: #ff5722;
            box-shadow: 2px 2px 5px rgba(0, 0, 0, 0.2);
        }

        .nav-item.active {
            color: #ff5722;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <h1>Explore Page</h1>
    <div class="nav-bar">
        <div class="nav-item" onclick="window.location.href='/home'">Home</div>
        <div class="nav-item active" onclick="window.location.href='/explore'">Explore</div>
        <div class="nav-item" onclick="window.location.href='/search'">Search</div>
        <div class="nav-item" onclick="window.location.href='/library'">Library</div>
    </div>
</body>
</html>
)rawliteral";

const char library_Page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>LED Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
            background: #e0e5ec;
            color: #333;
        }

        h1 {
            font-weight: 600;
            margin-bottom: 20px;
            color: #555;
        }

        .neumorphic {
            background: #e0e5ec;
            box-shadow: 9px 9px 16px #b8bcc2, -9px -9px 16px #ffffff;
            border-radius: 12px;
            border: none;
            outline: none;
        }
        .nav-bar {
            position: fixed;
            bottom: 0;
            width: 100%;
            display: flex;
            justify-content: space-evenly;
            background: #e0e5ec;
            padding: 10px 0;
            box-shadow: 0 -2px 5px rgba(0, 0, 0, 0.1);
            border-radius: 12px 12px 0 0;
        }

        .nav-item {
            flex: 1;
            text-align: center;
            color: #555;
            padding: 10px 0;
            transition: color 0.2s, box-shadow 0.2s;
            border-radius: 8px;
        }

        .nav-item:hover {
            color: #ff5722;
            box-shadow: 2px 2px 5px rgba(0, 0, 0, 0.2);
        }

        .nav-item.active {
            color: #ff5722;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <h1>Library Page</h1>
    <div class="nav-bar">
        <div class="nav-item" onclick="window.location.href='/home'">Home</div>
        <div class="nav-item active" onclick="window.location.href='/explore'">Explore</div>
        <div class="nav-item" onclick="window.location.href='/search'">Search</div>
        <div class="nav-item" onclick="window.location.href='/library'">Library</div>
    </div>
</body>
</html>
)rawliteral";

const char search_Page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>LED Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
            background: #e0e5ec;
            color: #333;
        }

        h1 {
            font-weight: 600;
            margin-bottom: 20px;
            color: #555;
        }

        .neumorphic {
            background: #e0e5ec;
            box-shadow: 9px 9px 16px #b8bcc2, -9px -9px 16px #ffffff;
            border-radius: 12px;
            border: none;
            outline: none;
        }
        .nav-bar {
            position: fixed;
            bottom: 0;
            width: 100%;
            display: flex;
            justify-content: space-evenly;
            background: #e0e5ec;
            padding: 10px 0;
            box-shadow: 0 -2px 5px rgba(0, 0, 0, 0.1);
            border-radius: 12px 12px 0 0;
        }

        .nav-item {
            flex: 1;
            text-align: center;
            color: #555;
            padding: 10px 0;
            transition: color 0.2s, box-shadow 0.2s;
            border-radius: 8px;
        }

        .nav-item:hover {
            color: #ff5722;
            box-shadow: 2px 2px 5px rgba(0, 0, 0, 0.2);
        }

        .nav-item.active {
            color: #ff5722;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <h1>Search Page</h1>
    <div class="nav-bar">
        <div class="nav-item" onclick="window.location.href='/home'">Home</div>
        <div class="nav-item active" onclick="window.location.href='/explore'">Explore</div>
        <div class="nav-item" onclick="window.location.href='/search'">Search</div>
        <div class="nav-item" onclick="window.location.href='/library'">Library</div>
    </div>
</body>
</html>
)rawliteral";
void setup() {
  Serial.begin(115200);

  // Configure LED strip
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  // Connect to WiFi
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  WiFi.softAP("ESP_Color_Control", "12345678");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.println("IP Address: " + WiFi.localIP().toString());

  // Serve pages
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("/home");
  });

  server.on("/home", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", colorPickerPage);
  });

  server.on("/explore", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", explorePage);
  });
  server.on("/search", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", search_Page);
  });
  server.on("/library", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", library_Page);
  });

  server.on("/setColor", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("color")) {
      String color = request->getParam("color")->value();
      setColor(color);
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/setBrightness", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("value")) {
      int brightness = request->getParam("value")->value().toInt();
      setBrightness(brightness);
    }
    request->send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  // No additional tasks in the loop
}

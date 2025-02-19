#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <WebSocketsServer.h>
#include <Ronin_SBUS.h>

// Access Point Setup
const char* AP_SSID = "RoninControl_Setup";
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);

// Server and DNS
ESP8266WebServer webServer(80);
DNSServer dnsServer;

// WebSockets Server
WebSocketsServer webSocket = WebSocketsServer(81); // Porta 81 per WebSockets

// Struct to save wifi config
struct WifiConfig {
    char wifi_ssid[32];
    char wifi_password[64];
    bool use_static_ip;
    IPAddress static_ip;
    IPAddress gateway;
    IPAddress subnet;
    uint8_t config_set;  // Flag per verificare se la configurazione è stata impostata
};

// Initialize wificonfig struct
WifiConfig wificonfig;

// Initialize SBUS interface
Ronin_SBUS mySBUS;

// Constants used to define min, mid and max values & frame delay
const int sbusMID         = 1024;   // Neutral val
const int sbusMIN         = 352;    // Min usable for analog val and switch val
const int sbusMAX         = 1696;   // Max usable for analog val and switch val
const int sbusWAIT        = 10;     // Frame timing delay in msecs

// Function to send logs throw WebSocket with timestamp, visible in webpage/log
void sendLog(String message) {
    // Get current time
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    
    // Timestamp format
    char timeString[20];
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    // Write formatted message
    String formattedMessage = String("[") + String(timeString) + "] " + message;
    webSocket.broadcastTXT(formattedMessage);
}

void setup() {
    // Inizialize EEPROM
    EEPROM.begin(512);
    // Initialize USB serial for debug
    // Serial.begin(9600); // Uncomment if you want to debug via serial console

    // Load saved wifi settings
    loadWifiConfig();

    // Initialize SBUS 
    mySBUS.begin();

    if (wificonfig.config_set != 1) {
        startSetupMode();
    } else {
        startNormalMode();
    }
}

// Function to load wifi config from the struct
void loadWifiConfig() {
    EEPROM.get(0, wificonfig);
    if (wificonfig.config_set != 1) {
        wificonfig.config_set = 0;
        wificonfig.use_static_ip = false;
        strcpy(wificonfig.wifi_ssid, ""); // Vuoto per configurazione
        strcpy(wificonfig.wifi_password, ""); // Vuoto per configurazione
        wificonfig.static_ip = IPAddress(192, 168, 1, 200);
        wificonfig.gateway = IPAddress(192, 168, 1, 1);
        wificonfig.subnet = IPAddress(255, 255, 255, 0);
    }
}

void saveWifiConfig() {
    EEPROM.put(0, wificonfig);
    EEPROM.commit();
}

// Function to start the ESP in setup mode --> generate open wifi ("RoninControl_Setup") to set up network settings
void startSetupMode() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(AP_SSID);
    dnsServer.start(DNS_PORT, "*", apIP);

    webServer.on("/", handleSetupRoot);
    webServer.on("/configure", handleConfigure);
    webServer.on("/reset", handleReset);
    webServer.onNotFound([]() { 
        webServer.sendHeader("Location", "/", true);
        webServer.send(302, "text/plain", ""); 
    });
    webServer.begin();
    sendLog("Setup mode started");
}

// Function to start the ESP in normal mode --> if it cannot connect within 25 attemps, it will restart in setup mode
void startNormalMode() {
    WiFi.mode(WIFI_STA);
    if (wificonfig.use_static_ip) {
        WiFi.config(wificonfig.static_ip, wificonfig.gateway, wificonfig.subnet);
    }
    WiFi.begin(wificonfig.wifi_ssid, wificonfig.wifi_password);
    sendLog("Connecting to wifi: " + String(wificonfig.wifi_ssid));
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 25) {
        delay(1000);
        sendLog("Connection attemp #" + attempts);
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        configTime(3600, 0, "pool.ntp.org", "time.nist.gov");

        sendLog("Connected to " + String(wificonfig.wifi_ssid));
        sendLog("IP: " + WiFi.localIP().toString());

        // Setting up WebServer routes
        webServer.on("/", handleStatusRoot);
        webServer.on("/configure", handleConfigure);
        webServer.on("/reset", handleReset);
        webServer.on("/log", handleLogPage);
        webServer.on("/wificonfig", handleWifiConfig);
        webServer.begin();

        // Initialize WebSocket
        webSocket.begin();
        webSocket.onEvent(webSocketEvent);
        sendLog("WebSocket Server started on port 81");
    } else {
        sendLog("Connection failed, starting setup mode");
        startSetupMode();
    }
}

// Setup page handling
void handleSetupRoot() {
    webServer.send(200, "text/html",
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "  <title>WiFi Configuration</title>"
        "  <style>"
        "    body { font-family: Arial, sans-serif; max-width: 600px; margin: auto; padding: 20px; }"
        "    input[type=text], input[type=password] { width: 100%; padding: 8px; margin: 5px 0; }"
        "    input[type=submit] { padding: 10px 20px; }"
        "  </style>"
        "</head>"
        "<body>"
        "  <h1>WiFi Configuration</h1>"
        "  <form id='wifiForm' action='/configure' method='POST'>"
        "    <label>SSID:</label><br>"
        "    <input type='text' name='ssid' required><br>"
        "    <label>Password:</label><br>"
        "    <input type='password' name='password' required><br>"
        "    <label>Use static IP:</label><br>"
        "    <input type='checkbox' name='static_ip' id='static_ip'><br>"
        "    <div id='static_ip_fields' style='display:none; margin-top:10px;'>"
        "      <label>IP:</label><br>"
        "      <input type='text' name='ip' placeholder='192.168.1.100'><br>"
        "      <label>Gateway:</label><br>"
        "      <input type='text' name='gateway' placeholder='192.168.1.1'><br>"
        "      <label>Subnet:</label><br>"
        "      <input type='text' name='subnet' placeholder='255.255.255.0'><br>"
        "    </div>"
        "    <br>"
        "    <input type='submit' value='Save'>"
        "  </form>"
        "  <script>"
        "    document.getElementById('static_ip').addEventListener('change', function() {"
        "      document.getElementById('static_ip_fields').style.display = this.checked ? 'block' : 'none';"
        "    });"
        "    document.getElementById('wifiForm').addEventListener('submit', function(event) {"
        "      alert('Configuration Saved!\\nThe device will reboot and try to connect to WiFi.\\nIf something goes wrong, it will restart in setup mode.');"
        "    });"
        "  </script>"
        "</body>"
        "</html>"
    );
}

// Status page handling
void handleStatusRoot() {
    String page = "<!DOCTYPE html>"
                  "<html>"
                  "<head>"
                  "<title>Connection Status</title>"
                  "<style>"
                  "  body { font-family: Arial, sans-serif; text-align: center; padding: 20px; background-color: #f4f4f4; }"
                  "  .container { max-width: 400px; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }"
                  "  h1 { color: #333; }"
                  "  p { font-size: 18px; }"
                  "  .button { display: block; width: 100%; padding: 10px; margin: 10px 0; border: none; border-radius: 5px; font-size: 18px; text-decoration: none; cursor: pointer; text-align: center; }"
                  "  .btn-log { background-color: #007bff; color: white; }"
                  "  .btn-config { background-color: #28a745; color: white; }"
                  "  .btn-reset { background-color: #dc3545; color: white; }"
                  "</style>"
                  "</head>"
                  "<body>"
                  "<div class='container'>"
                  "  <h1>Connection Status</h1>"
                  "  <p><strong>SSID:</strong> " + String(WiFi.SSID()) + "</p>"
                  "  <p><strong>IP:</strong> " + WiFi.localIP().toString() + "</p>"
                  "  <button class='button btn-log' onclick='logRedirect()'>View Log</button>"
                  "  <button class='button btn-config' onclick='configRedirect()'>WiFi Settings</button>"
                  "  <button class='button btn-reset' onclick='confirmReset()'>Reset WiFi Config</button>"
                  "</div>"
                  "<script>"
                  "  function confirmReset() {"
                  "    if (confirm('WiFi configuration will be reset and the device will reboot in setup mode. Continue?')) {"
                  "      setTimeout(() => { window.location.href = '/reset'; }, 500);"
                  "      setTimeout(() => { window.close(); }, 1000);"
                  "    }"
                  "  }"
                  "  function logRedirect() {"
                  "     window.location.href = '/log';"
                  "  }"
                  "  function configRedirect() {"
                  "     window.location.href = '/wificonfig';"
                  "  }"
                  "</script>"
                  "</body>"
                  "</html>";

    webServer.send(200, "text/html", page);
}

void handleWifiConfig() {
    String page = "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "  <title>WiFi Configuration</title>"
        "  <style>"
        "    body { font-family: Arial, sans-serif; max-width: 600px; margin: auto; padding: 20px; }"
        "    input[type=text], input[type=password] { width: 100%; padding: 8px; margin: 5px 0; }"
        "    input[type=submit] { padding: 10px 20px; }"
        "  </style>"
        "</head>"
        "<body>"
        "  <h1>WiFi Configuration</h1>"
        "  <form id='wifiForm' action='/configure' method='POST'>"
        "    <label>SSID:</label><br>"
        "    <input type='text' name='ssid' value='" + String(wificonfig.wifi_ssid) + "' disabled><br>"
        "    <div id='static_ip_fields' style='display:block; margin-top:10px;'>"
        "      <label>IP:</label><br>"
        "      <input type='text' name='ip' value='" + WiFi.localIP().toString() + "' required><br>"
        "      <label>Gateway:</label><br>"
        "      <input type='text' name='gateway' value='" + WiFi.gatewayIP().toString() + "' required><br>"
        "      <label>Subnet:</label><br>"
        "      <input type='text' name='subnet' value='" + WiFi.subnetMask().toString() + "' required><br>"
        "    </div>"
        "    <br>"
        "    <input type='submit' value='Save'>"
        "  </form>"
        "  <script>"
        "    document.getElementById('wifiForm').addEventListener('submit', function(event) {"
        "      alert('Configuration Saved!\\nThe device will reboot and try to connect to WiFi.\\nIf something goes wrong, it will restart in setup mode.');"
        "    });"
        "  </script>"
        "</body>"
        "</html>";

    webServer.send(200, "text/html", page);
}

// Log page handling
void handleLogPage() {
    String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Messages Log</title>
    <style>
        body { font-family: Arial, sans-serif; }
        #logs { width: 100%; height: 80vh; border: 1px solid #ccc; overflow-y: scroll; padding: 10px; background-color: #f9f9f9; }
        #clearButton { margin-top: 10px; padding: 10px 20px; }
        button { cursor: pointer; }
    </style>
</head>
<body>
    <h1>Messages Log</h1>
    <div id="logs"></div>
    <button id="clearButton">Cancella Log</button>

    <script>
        let socket = new WebSocket('ws://' + location.hostname + ':81/');
        let logs = document.getElementById('logs');
        let clearButton = document.getElementById('clearButton');

        socket.onopen = function(event) {
            logs.innerHTML += '<p><em>Connection estabilished.</em></p>';
        };

        socket.onmessage = function(event) {
            logs.innerHTML += '<p>' + event.data + '</p>';
            logs.scrollTop = logs.scrollHeight; // Scorri automaticamente verso il basso
        };

        socket.onclose = function(event) {
            logs.innerHTML += '<p><em>Connection closed.</em></p>';
        };

        clearButton.addEventListener('click', function() {
            logs.innerHTML = '';
            // Optional: Send a message to server when clear button is pressed
            // socket.send('clear');
        });
    </script>
</body>
</html>
)rawliteral";

    webServer.send(200, "text/html", page);
}

// WiFi Configuration handling (receives data from POST form)
void handleConfigure() {
    // Form Data
    String ssid = webServer.arg("ssid");
    String password = webServer.arg("password");
    bool use_static_ip = webServer.hasArg("static_ip");
    String ip = webServer.arg("ip");
    String gateway = webServer.arg("gateway");
    String subnet = webServer.arg("subnet");

    if(wificonfig.config_set == 0){
      if (ssid.length() > 0 && password.length() > 0) {
        strncpy(wificonfig.wifi_ssid, ssid.c_str(), sizeof(wificonfig.wifi_ssid));
        strncpy(wificonfig.wifi_password, password.c_str(), sizeof(wificonfig.wifi_password));
        wificonfig.use_static_ip = use_static_ip;
        if (use_static_ip) {
            wificonfig.static_ip = parseIPAddress(ip);
            wificonfig.gateway = parseIPAddress(gateway);
            wificonfig.subnet = parseIPAddress(subnet);
        }
        wificonfig.config_set = 1;
      } else {
        webServer.send(400, "text/plain", "SSID and Password are required.");
        return;
      }
    }else{
      if (ip.length() > 0 && gateway.length() > 0 && subnet.length() > 0) {
        wificonfig.static_ip = parseIPAddress(ip);
        wificonfig.gateway = parseIPAddress(gateway);
        wificonfig.subnet = parseIPAddress(subnet);
        wificonfig.use_static_ip = true;
      } else {
        webServer.send(400, "text/plain", "IP Address, Gateway and Subnet are required.");
        return;
      }
    }
      
      saveWifiConfig();
      sendLog("WiFi config saved. Reboot...");
      webServer.sendHeader("Location", "/", true);
      webServer.send(302, "text/plain", "WiFi config saved. Reboot...");
      delay(1000);
      ESP.restart();
}

// Parsing IP from string
IPAddress parseIPAddress(String ip) {
    IPAddress result;
    result.fromString(ip);
    return result;
}

// Handling reset via web
void handleReset() {
    // Configurations reset
    memset(&wificonfig, 0, sizeof(wificonfig));
    saveWifiConfig();
    sendLog("WiFi config resetted. Reboot...");
    webServer.sendHeader("Location", "/", true);
    webServer.send(302, "text/plain", "WiFi config resetted. Reboot...");
    delay(1000);
    ESP.restart();
}

// Handling WebSocket events
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            sendLog("Client WebSocket " + String(num) + " Disconnected");
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            sendLog("Client " + String(num) + " connected from " + String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]));
            break;
        }
        case WStype_TEXT:
            sendLog("WebSocket message received: " + String((char*)payload));
            handleWebSocketMessage((char*)payload);
            break;
    }
}

// Gestione dei messaggi WebSocket
void handleWebSocketMessage(char* msg) {
    // Message is supposed to be in the format "ch1:1600"
    // Es. "ch1:1600"

    // Convert message in string
    String message = String(msg);

    // Verify if message contains the separator ":"
    int separatorIndex = message.indexOf(':');
    if (separatorIndex == -1) {
        sendLog("Message format not valid!");
        return;
    }

    // Extract the channel number and its value
    int channel = message.substring(2, separatorIndex).toInt();
    int value = message.substring(separatorIndex + 1).toInt();

    // Control if value is in the allowed range
    if (value < sbusMIN || value > sbusMAX) {
        sendLog("Value out of allowed range!");
        return;
    }

    // Control if channel number is in the allowed range
    if (channel < 0 || channel > 16) {
        sendLog("Channel number out of allowed range!");
        return;
    }

    // print in logs received value
    sendLog("Channel:" + String(channel) +", Value: " + String(value));

    mySBUS.SetValue(channel, value);
    mySBUS.Update();
    mySBUS.Send(); // Send S-BUS frame to Ronin
    delay(sbusWAIT); // Wait for S-BUS frame transmission

}

void loop() {
    dnsServer.processNextRequest();
    webServer.handleClient();
    webSocket.loop(); // Manage WebSocket comunications
}

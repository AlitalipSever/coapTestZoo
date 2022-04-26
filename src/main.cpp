#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>

#include <ArduinoJson.h>

const char* ssid     = "karakoyun";
const char* password = "Ultra-Begu14";

// CoAP client response callback
void callback_response(CoapPacket &packet, IPAddress ip, int port);

// CoAP server endpoint url callback
void callback_light(CoapPacket &packet, IPAddress ip, int port);

// CoAP server endpoint url callback
void callback_light_off(CoapPacket &packet, IPAddress ip, int port);

// UDP and CoAP class
WiFiUDP udp;
Coap coap(udp);

// 192.168.0.1 router
// 192.168.0.184 esp32
// Set your Static IP address
IPAddress local_IP(192, 168, 167, 233);
// Set your Gateway IP address
IPAddress gateway(192, 168, 167, 1);
// IPAddress gateway(192, 168, 43, 1);
// 0000


IPAddress subnet(255, 255, 255, 0);
// 255, 255, 255, 0
IPAddress primaryDNS(0, 0, 0, 0);   //optional
IPAddress secondaryDNS(0, 0, 0, 0); //optional

// LED STATE
bool LEDSTATE;

// CoAP server endpoint URL
void callback_light(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("coap message safe mode:");
  StaticJsonDocument<96> aimpointBuffer;                                   // https://arduinojson.org/v6/assistant/ for memory calc.
  // send response
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = '\0';                                           // null type error fixed
  // Serial.println(p);
  DeserializationError error = deserializeJson(aimpointBuffer, p);        // p ==> input
  JsonObject obj = aimpointBuffer.as<JsonObject>();
  // hs->aimpoint(0) = obj["x"];
  // hs->aimpoint(1) = obj["y"];  // fixed cleaning 
  // hs->aimpoint(2) = obj["z"];
  int x_angle = obj["x_angle"];
  int y_angle = obj["y_angle"];
  Serial.println(x_angle);
  Serial.println(y_angle);
    // Deserialize the JSON document
    if (error)
    Serial.println(F("Failed to read input, using default configuration"));
    switch (error.code()) {
    case DeserializationError::Ok:
        Serial.print(F("Deserialization succeeded"));
         Serial.println(" ");
        break;
    case DeserializationError::InvalidInput:
        Serial.print(F("Invalid input!"));
        break;
    case DeserializationError::NoMemory:
        Serial.print(F("Not enough memory"));
        break;
    default:
        Serial.print(F("Deserialization failed"));
        break;
}
 coap.sendResponse(ip, port, packet.messageid);
}

// CoAP server endpoint URL
void callback_light_off(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("[Light] OFF");
  
  // send response
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  
  String message(p);
  Serial.println(p);

  coap.sendResponse(ip, port, packet.messageid);

}

// CoAP client response callback
void callback_response(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("[Coap Response got]");
  
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  
  Serial.println(p);
}

void setup() {
  Serial.begin(9600);

  Serial.println("gateway: ");
  Serial.println(gateway);

  
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }


  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }


  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // LED State
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  LEDSTATE = true;
  
  // add server url endpoints.
  // can add multiple endpoint urls.
  // exp) coap.server(callback_switch, "switch");
  //      coap.server(callback_env, "env/temp");
  //      coap.server(callback_env, "env/humidity");
  coap.server(callback_light, "safe_mode");
  Serial.println("Setup Callback safe_mode");
  coap.server(callback_light_off, "light_off");

  // client response callback.
  // this endpoint is single callback.
  Serial.println("Setup Response Callback");
  coap.response(callback_response);

  // start coap server/client
  coap.start();
}

void loop() {
  delay(1000);
  coap.loop();
}
/*
if you change LED, req/res test with coap-client(libcoap), run following.
coap-client -m get coap://(arduino ip addr)/light
coap-client -e "1" -m put coap://(arduino ip addr)/light
coap-client -e "0" -m put coap://(arduino ip addr)/light
*/
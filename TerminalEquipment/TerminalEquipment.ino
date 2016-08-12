

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

const char* host = "esp8266";
const char* ssid = "124-2F";
const char* password = "2883153";
const char* mqtt_server = "192.168.1.108";
const char* topic = "home.liveroom.tv";
const char* apssid = "MoonAP";
const char* passphrase = "12345678";
String st;
String content;
int statusCode;

WiFiServer  server(80);
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[23];

int wifimode = 0;

void setup() {
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
  //Serial.print(digitalRead(2));
  Serial.begin(9600);
  server.begin();

  if (testWifi()) {
    wifimode = 0;
    WiFi.mode(WIFI_STA);
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    setup_dnsserver();
  } else {
    //WiFi.disconnect();
    wifimode = 1;
    WiFi.mode(WIFI_AP);
    Serial.println("WiFi AP MODE");
    WiFi.softAP(topic, passphrase, 6, 0);
    ScanNetwork();
  };
  //setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}



void setup_dnsserver() {
  if (!MDNS.begin(topic)) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  MDNS.addService("http", "tcp", 80);
  Serial.print("You can now connect to http://");
  Serial.print(topic);
  Serial.println(".local");
}

void setup_wifi() {
  int c = 0;
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while ( c < 15 ) {
    if (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    c++;
  }
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED) {

    wifimode = 0;
    WiFi.mode(WIFI_STA);
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    //WiFi.disconnect();
    wifimode = 1;
    WiFi.mode(WIFI_AP);
    Serial.println("WiFi AP MODE");
    WiFi.softAP(topic, passphrase, 6, 0);
    ScanNetwork();
  }

  Serial.println("");

}

bool testWifi(void) {
  int c = 0;
  Serial.println("Waiting for Wifi to connect...");
  while ( c < 10 ) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(1000);
    Serial.print(".");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, ESP8266 change to AP mode");
  Serial.println("");
  return false;
}





void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  ///if ((char)payload[0] == '1') {
  digitalWrite(2, (char)payload[0]);
  //} else {
  //  digitalWrite(2, 0);
  // }

}

void reconnect() {
  while (!client.connected()) {
    if (client.connect(topic)) {
      client.subscribe(topic);
    } else {
      delay(5000);
    }
  }
}


void send_light_state() {
  long now = millis();
  char json[50] = "{\"home.liveroom.tv\":\"1\"}" ;
  if (now - lastMsg > 5000) {
    lastMsg = now;

    //sprintf(msg, "%d", digitalRead(2));
    json[26] = (char)digitalRead(2);
    client.publish("home.light.state", json );
    //if (digitalRead(2) == HIGH) {

    //} else {
    //  client.publish("home.light.state", json);
    //}
  }
}


void createhttp() {
  WiFiClient wifiClient = server.available();
  if (!wifiClient) {
    return;
  }
  Serial.println("");
  Serial.println("New client");  //有偵測到任何連線到server的, 就顯示新的client連線
  while (wifiClient.connected() && !wifiClient.available()) {
    delay(1);
  }
  String req = wifiClient.readStringUntil('\r'); //ESP8266讀取從我的電腦發送的GET / HTTP/1.1(http request)
  Serial.println(req);

  int addr_start = req.indexOf(' '); //找出' '(空格)在req中的位置
  Serial.print("addr_start = ");
  Serial.println(addr_start);
  int addr_end = req.indexOf(' ', addr_start + 1); //找出第二個空格在req中的位置
  Serial.print("addr_end = ");
  Serial.println(addr_end);
  if (addr_start == -1 || addr_end == -1) {     //如果都找不到, 會得到-1
    Serial.print("Invalid request: ");
    Serial.println(req);
    return;
  }
  req = req.substring(addr_start + 1, addr_end); //讀取從addr_start+1(4)到addr_end(5), 如果原字串內容為"GET / HTTP/1.1", 則會讀取到"/"這個字串
  Serial.print("Request: ");
  Serial.println(req);
  wifiClient.flush();

  String s;
  if (req == "/") {
    IPAddress ip = WiFi.localIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html><form action='setting'>";
    if (wifimode == 1)
      s += "網路環境設定:" + st + "<input type='submit' value='設定' />";
    s += "</form></html>\r\n\r\n";
    //Serial.println("Sending 200");
  }
  if (req == "/setting") {

  }


  //else {
  //  s = "HTTP/1.1 404 Not Found\r\n\r\n"; //404代碼是指找不到網頁資料
  //  Serial.println("Sending 404");
  //}
  wifiClient.print(s);
  Serial.println("Done with client");
}

void ScanNetwork() {


  int n = WiFi.scanNetworks();
  Serial.print("Scan Network Done...and ");
  if (n == 0)
    Serial.println("No Any Networks Found!");
  else
  {
    Serial.print(n);
    Serial.println(" Networks Found!");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      byte encryption = WiFi.encryptionType(i);
      Serial.print(" Encryption Type:");
      //Serial.println(encryption,HEX);  // TKIP (WPA) = 2 , WEP = 5 , CCMP (WPA) = 4 , NONE = 7 , AUTO = 8(WPA or WPA2)
      switch (encryption) {
        case 2: Serial.println("TKIP(WPA)"); break;
        case 5: Serial.println("WEP"); break;
        case 4: Serial.println("CCMP(WPA)"); break;
        case 7: Serial.println("NONE"); break;
        case 8: Serial.println("AUTO(WPA or WPA2)"); break;
      }
      //Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(100);
    }
  }
  Serial.println("");
  String k;
  st = "<select>";
  for (int i = 0; i < n; ++i)
  {
    st += "<option value=" + WiFi.SSID(i) + " >" + WiFi.SSID(i) + "</option>";

  }
  st += "</select><select>";


  st += "<opeion value='2'>TKIP(WPA)</opeion>";
  st += "<opeion value='5'>WEP</opeion>";
  st += "<opeion value='4'>CCMP(WPA)</opeion>";
  st += "<opeion value='7'>NONE</opeion>";
  st += "<opeion value='8'>AUTO(WPA or WPA2)</opeion>";

  st += "</select>";
}

void loop() {


  if (wifimode == 0) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    send_light_state();
  }
  createhttp();
}

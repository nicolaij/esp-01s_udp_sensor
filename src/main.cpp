#include <Arduino.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>

#include <GyverHTU21D.h>
GyverHTU21D htu;

#include <EEPROM.h>

#include <ESP8266HTTPClient.h>

// Set WiFi credentials
#define WIFI_SSID "Nadtocheeva 5"
#define WIFI_PASS "123123123"

IPAddress send_IP(192, 168, 10, 20);

// UDP
WiFiUDP UDP;

char packet[255];

// Set your Static IP address
IPAddress local_IP(192, 168, 10, 22);
// Set your Gateway IP address
IPAddress gateway(192, 168, 10, 1);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 10, 1); // optional
IPAddress secondaryDNS(8, 8, 4, 4);    // optional

IPAddress host(192, 168, 10, 20);

int hostport = 12345;

ADC_MODE(ADC_VCC);

uint32_t a;

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(512);
  EEPROM.get(0, a);
  local_IP = IPAddress(a);
  EEPROM.get(16, a);
  gateway = IPAddress(a);
  EEPROM.get(32, a);
  subnet = IPAddress(a);
  EEPROM.get(48, a);
  host = IPAddress(a);
  EEPROM.get(64, hostport);

  WiFiManager wifiManager;

  // wifiManager.resetSettings();

  WiFiManagerParameter customLocalIp("localIP", "Local IP", local_IP.toString().c_str(), 16);
  WiFiManagerParameter customLocalMask("localMask", "Local Mask", subnet.toString().c_str(), 16);
  WiFiManagerParameter customLocalGw("localGw", "Local Gateway", gateway.toString().c_str(), 16);

  wifiManager.addParameter(&customLocalIp);
  wifiManager.addParameter(&customLocalMask);
  wifiManager.addParameter(&customLocalGw);

  WiFiManagerParameter customHostServer("defaultHost", "Host Server", host.toString().c_str(), 16);
  WiFiManagerParameter customHostPort("defaultHostPort", "Port Host Server", String(hostport).c_str(), 5);

  wifiManager.addParameter(&customHostServer);
  wifiManager.addParameter(&customHostPort);

  wifiManager.setSTAStaticIPConfig(local_IP, gateway, subnet);

  bool res = wifiManager.autoConnect();
  if (!res)
  {
    Serial.println("Failed to connect");
    ESP.restart();
  };

  // store updated parameters
  IPAddress addr;
  bool ee_update = false;

  if (addr.fromString(customLocalIp.getValue()))
    if (local_IP != addr)
    {
      local_IP = addr;
      a = addr;
      EEPROM.put(0, a);
      Serial.print("New IP: ");
      Serial.println(addr.toString());
      ee_update = true;
    }

  if (addr.fromString(customLocalMask.getValue()))
    if (subnet != addr)
    {
      subnet = addr;
      a = addr;
      EEPROM.put(32, a);
      Serial.print("New mask: ");
      Serial.println(addr.toString());
      ee_update = true;
    }

  if (addr.fromString(customLocalGw.getValue()))
    if (gateway != addr)
    {
      gateway = addr;
      a = addr;
      EEPROM.put(16, a);
      Serial.print("New gateway: ");
      Serial.println(addr.toString());
      ee_update = true;
    }

  if (addr.fromString(customHostServer.getValue()))
    if (host != addr)
    {
      host = addr;
      a = addr;
      EEPROM.put(48, a);
      Serial.print("New host: ");
      Serial.println(addr.toString());
      ee_update = true;
    }

  int hp = String(customHostPort.getValue()).toInt();
  if (hp != hostport)
  {
    hostport = hp;
    EEPROM.put(64, hp);
    Serial.print("New port host: ");
    Serial.println(hp);
    ee_update = true;
  }

  if (ee_update == true)
    EEPROM.commit();

  if (!htu.begin(0, 2))
    Serial.println(F("HTU21D error"));

  float temp = htu.getTemperatureWait();
  float hum = htu.getHumidityWait();

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.println(" *C");

  Serial.print("Hum: ");
  Serial.print(hum);
  Serial.println(" %");

  Serial.println();

  float volts = ESP.getVcc();

  Serial.print("Volts: ");
  Serial.print(volts);
  Serial.println(" v");

  // int nVoltageRaw = analogRead(A0);
  // float fVoltage = (float)nVoltageRaw * 1;

  sprintf(packet, "{\"t\":%f,\"h\":%f,\"v\":%f}", temp, hum, volts / 1000);

  // int max = 100;
  /*  while (WiFi.status() != WL_CONNECTED)
    {
      delay(100);
      if (max-- == 0)
      {
        Serial.println("No connect");

        break;
      }
    }
    delay(1000);
  */
  // Use WiFiClient class to create TCP connections
  /*
    WiFiClient client;
    HTTPClient http;
    http.begin(client, host.toString(), 80, "/number/floor_1_sensor_temperature_data/set?value=" + String(temp * 10));
    int httpCode = http.POST("");
    Serial.print("POST httpCode: ");
    Serial.println(httpCode);
    http.end();

    http.begin(client, host.toString(), 80, "/number/floor_1_sensor_humidity_data/set?value=" + String(hum));
     httpCode = http.POST("");
    Serial.print("POST httpCode: ");
    Serial.println(httpCode);
    http.end();

    http.begin(client, host.toString(), 80, "/number/floor_1_sensor_vcc_data/set?value=" + String(volts));
     httpCode = http.POST("");
    Serial.print("POST httpCode: ");
    Serial.println(httpCode);
    http.end();
  */
  // httpCode will be negative on error
  /*int r1;
  int n = 1;
  do
  {
    delay(500);

    UDP.beginPacket(host, hostport);
    UDP.write(packet);
    r1 = UDP.endPacket();

    Serial.print("endPacket: ");
    Serial.println(r1);

  } while (n--);
*/

  // TCP
  WiFiClient client;
  client.setTimeout(2000);

  Serial.print("Connect: ");
  Serial.println(client.connect(host, hostport));

  if (client.connected())
  {
    client.print(packet);
    client.flush(1000);
    client.stop(1000);
  };

  Serial.print("Go sleep: ");
  Serial.println(millis());
  ESP.deepSleep(30e6);
}

void loop()
{
  // put your main code here, to run repeatedly:
}

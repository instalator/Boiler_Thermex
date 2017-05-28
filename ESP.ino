#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "******";
const char* password = "******";
const char* mqtt_server = "192.168.1.1"; //Сервер MQTT

IPAddress ip(192,168,1,241);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WiFiClient espClient;
PubSubClient client(espClient);

#define ID_CONNECT "Boiler"
#define LED     2
byte data[10] = {};
int temp;
int prsv;
int dbl;
int sngl;
int pwr;
bool power;
int temp_prev;
int prsv_prev;
int dbl_prev;
int sngl_prev;
bool power_prev;
char b[2];


void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    digitalWrite(LED, !digitalRead(LED));
  }
  digitalWrite(LED, LOW);
}

void reconnect() {
  digitalWrite(LED, !digitalRead(LED));
  while (!client.connected()) {
    if (client.connect(ID_CONNECT)) {
      client.publish("myhome/Boiler/connection", "true");
      client.publish("myhome/Boiler/temperature", "");
      client.publish("myhome/Boiler/power", "false");
      client.publish("myhome/Boiler/single_power", "false");
      client.publish("myhome/Boiler/double_power", "false");
      client.publish("myhome/Boiler/preservation", "false");
      client.publish("myhome/Boiler/power_selector", "false");
      client.publish("myhome/Boiler/temp_selector", "false");
      client.subscribe("myhome/Boiler/#");
      digitalWrite(LED, LOW);
    } else {
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String strTopic = String(topic);
  String strPayload = String((char*)payload);
  ///////////
  if (strTopic == "myhome/Boiler/temperature"){
    int t = strPayload.toInt();
    if (t == 35 || t == 40 || t == 45 || t == 50 || t == 55 || t == 60 || t == 65 || t == 70 || t == 75){
      Serial.print("cmd");
      Serial.println(t);
    }
  }
  //////////
  if (strTopic == "myhome/Boiler/power"){
    if (strPayload == "true" && power == 0){
        Serial.println("cmd1");
    } else if(strPayload == "false" && power == 1){
        Serial.println("cmd1");
    }
  }
  //////////
  if (strTopic == "myhome/Boiler/power_selector"){
    if (strPayload == "true" && power == 1){
        Serial.println("cmd2");
        client.publish("myhome/Boiler/power_selector", "false");
    }
  }
  //////////
  if (strTopic == "myhome/Boiler/temp_selector"){
    if (strPayload == "true"){
        Serial.println("cmd3");
        client.publish("myhome/Boiler/temp_selector", "false");
    }
  }
}

void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(19200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  delay(20000);
  if(Serial.available() > 0){
    temp = Serial.parseInt();
    prsv = Serial.parseInt();
    dbl =  Serial.parseInt();
    sngl = Serial.parseInt();
    pwr =  Serial.parseInt();
    
    if (pwr > 0){
      power = true;
    } else {
      power = false;
    }
    if (Serial.read() == ':') {
      if(temp != temp_prev || prsv != prsv_prev || dbl != dbl_prev || sngl != sngl_prev || power != power_prev){
          temp_prev = temp;
          prsv_prev = prsv;
          dbl_prev = dbl;
          sngl_prev = sngl;
          power_prev = power;
          char b[5]; 
          String char_temp = String(temp);
          char_temp.toCharArray(b,5);
          client.publish("myhome/Boiler/temperature", b);
          client.publish("myhome/Boiler/preservation", IntToBool(prsv));
          client.publish("myhome/Boiler/double_power", IntToBool(dbl));
          client.publish("myhome/Boiler/single_power", IntToBool(sngl));
          client.publish("myhome/Boiler/power", IntToBool(power));
      }
    }
  }
  if (!client.connected()){
    reconnect();
  }
  client.loop();
}

char* IntToBool (int r) {
    if (r > 0){
      return "true";
    } else{
      return "false";
    }
}

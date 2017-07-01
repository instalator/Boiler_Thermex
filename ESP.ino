#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "xxx";
const char* password = "xxx";
const char* mqtt_server = "192.168.1.190"; //Сервер MQTT

IPAddress ip(192,168,1,241);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WiFiClient espClient;
PubSubClient client(espClient);

#define ID_CONNECT "Boiler"
#define LED     2
byte data[10] = {};
int temp = 0;
int prsv = 0;
int dbl = 0;
int sngl = 0;
int pwr = 0;
bool power = true;
int temp_prev = 0;
int prsv_prev = 0;
int dbl_prev = 0;
int sngl_prev = 0;
bool power_prev = false;
char b[5];
String inputString = "";
boolean stringComplete = false;


void setup_wifi() {
  delay(10);
  WiFi.mode(WIFI_STA);
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
char* IntToBool (int r) {
    if (r > 0){
      return "true";
    } else{
      return "false";
    }
}

void loop() {
  if (!client.connected()){
    reconnect();
  }
  client.loop();
  if(Serial.available() > 0){
    char inChar = (char)Serial.read(); 
    inputString += inChar;
    if (inChar == ':') {
      stringComplete = true;
      ParseStr();
    } 
  }
}

void ParseStr(){
  String str = inputString;
  if (stringComplete == true){
    temp = str.substring(0, 2).toInt();
    prsv = str.substring(3, 4).toInt();
    dbl =  str.substring(5, 6).toInt();
    sngl = str.substring(7, 8).toInt();
    pwr =  str.substring(9, 10).toInt();

    if (pwr > 0){
      power = true;
    } else {
      power = false;
    }

    if(prsv != prsv_prev || dbl != dbl_prev || sngl != sngl_prev || power != power_prev || temp >= (temp_prev + 2) || temp <= (temp_prev - 2)){
          temp_prev = temp;
          prsv_prev = prsv;
          dbl_prev = dbl;
          sngl_prev = sngl;
          power_prev = power;
          
          String char_temp = String(temp);
          char_temp.toCharArray(b,5);
          client.publish("myhome/Boiler/temperature", b);
          client.publish("myhome/Boiler/preservation", IntToBool(prsv));
          client.publish("myhome/Boiler/double_power", IntToBool(dbl));
          client.publish("myhome/Boiler/single_power", IntToBool(sngl));
          client.publish("myhome/Boiler/power", IntToBool(power));
    }
    inputString = "";
    stringComplete = false;
  }
}

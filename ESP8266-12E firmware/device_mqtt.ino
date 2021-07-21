#include "Arduino.h"
#include "ArduinoJson.h"
#include "ESP8266WiFi.h"
#include "String.h"
#include "stdlib.h"
#include "PubSubClient.h"

#define TASK_PIN 4
#define MAX_MSG_LEN 256
#define LED 16

void send_data();
void Data_processing();
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnecting();
byte c, CRC1, CRC2;
char RxBuffer[13];
String readstring;
int indx, ppm, bpm;
float dust;
int i = 0;
bool state;
unsigned long t;

StaticJsonDocument<256> doc;

const char* ssid = "P0729";
const char* password = "77296729";

const uint16_t hiveMQ_port = 1883; // my port
const char* hiveMQ_host = "broker.mqttdashboard.com";
const char* username = "hppzbr";
const char* pw = "Lailano01";
/*
  device1 clientID = clientId-vBfbWRr4qq
  device2 clientID = clientId-WFLbiZieFy
  device3 clientID = clientId-A6Qzrw3o9R
*/
const char* clientID ="clientId-A6Qzrw3o9R";
const char* topic = "myHiveMQ/device3/topic";


WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  setup_wifi();
  client.setServer(hiveMQ_host, hiveMQ_port);
  client.setCallback(callback);
}

void loop() {
  // put your main code here, to run repeatedly:
//  while(state){
  Data_processing();
  delay(1000);
//  state = digitalRead(TASK_PIN);
 // }
 // state = digitalRead(TASK_PIN);
  if(!client.connected())//check connection
    reconnecting();
  client.loop();
  if(millis() - t > 4000){//wait 500ms
    t = millis();
    Serial.print("Published data to device1 topic ");
    send_data();
  }

}

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(50);
    Serial.print(".");
  }
  Serial.println(".");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


//call back function de nhan du lieu
void callback(char* topic, byte* payload, unsigned int length){
  char str[length + 1];
  Serial.println("message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  int i = 0;
  for (i=0; i<length; i++){
    Serial.print((char)payload[i]);
    str[i] = (char)payload[i];
  }
  str[i] = 0;//Null termination
  Serial.println();
  StaticJsonDocument <256> doc;
  deserializeJson(doc, payload);
  float Dust = doc["Dust"];
  float eCO2 = doc["eCO2"];
  float eTVOC = doc["eTVOC"];  
  Serial.println("Dust =");
  Serial.print(Dust);
  Serial.println("eCO2 =");
  Serial.print(eCO2);
  Serial.println("eTVOC =");
  Serial.print(eTVOC);  
//  deserializeJson(JSON_data, (const byte*)payload, length);
//
//  Serial.print("Co tin nhan tu topic:");
//  Serial.print(topic);
//  for (int i = 0; i< length; i++)
//    Serial.print((char)payload[i]);
//  Serial.println(".");
}

//reconect functon khi mat ket noi voi MQTT Broker
void reconnecting(){
  while(!client.connected()){//wait until connected
    //thuc hien ket noi voi hiveMQ
    if (client.connect(clientID,username,pw,topic,0,0,"offline"))  //kết nối vào broker
    {
      Serial.println("Đã kết nối:");
      client.subscribe(topic); //dang ky vao topic myHiveMQ
    }
    else 
    {
      Serial.print("Lỗi:, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Đợi 5s
      delay(5000);
    }
  }
}

void send_data(){
  doc["Dust"] = dust;
  doc["eCO2"] = ppm;
  doc["eTVOC"] = bpm;
  char out[128];
  int b = serializeJson(doc, out);
  Serial.println("bytes = ");
  Serial.println(b, DEC);
  boolean rc = client.publish(topic, out);
  
}

void Data_processing(){
    while(Serial.available()>0){

    c = Serial.read();
    if((c=='\n') && (RxBuffer[indx-1] =='\r')){
    Data_processing();
//    Serial.print(RxBuffer);
    c = 0;
//    readstring ="";
//    RxBuffer =0;
    indx = 0;
      break;
    }
    else{
    if(indx < 13){
//      buff[indx] = c;//save data in the next index in the array buffer
      RxBuffer[indx] = c;
      indx++;
      }
    }
  }  
  CRC1 = RxBuffer[1] + RxBuffer[2] + RxBuffer[3] + RxBuffer[4] ;
  CRC2 = RxBuffer[7] + RxBuffer[8] + RxBuffer[9] + RxBuffer[10] ;  

  if((RxBuffer[0] == 0xDD) && (CRC1 == RxBuffer[5])){
    //processing dust data
    dust = RxBuffer[1]*16*16*16 + RxBuffer[2]*16*16 + RxBuffer[3]*16 + RxBuffer[4];
    dust = dust/100.0;
  }
  
  if((RxBuffer[6] == 0xCC) && (CRC2 == RxBuffer[11])){
    //processing IAQ data
    ppm = RxBuffer[7]*16 + RxBuffer[8];
    bpm = RxBuffer[9]*16 + RxBuffer[10];
  }
  doc["Dust"] = dust;
  doc["eCO2"] = ppm;
  doc["eTVOC"] = bpm;
  char out[128];
  int b = serializeJson(doc, out);
//  Serial.println("bytes = ");
  Serial.println(out);
}

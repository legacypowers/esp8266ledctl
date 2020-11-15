
#include <ESP8266WiFi.h>

#define STASSID "WIFISSD"
#define STAPSK  "WIFIPASSWORD"
#define MAX_SRV_CLIENTS 2
const char* ssid = STASSID;
const char* password = STAPSK;
const int port = 1337;


uint8_t newPacket = 0,lastStrobe = 0,fadeStep = 0,onOff, buf[5],curStatus[3]; //Buffer do pacote 0 = modo | 1 = R |  2 = G | 3 = B | 4 = intervalo

typedef struct{
    char ssid[32];
    char password[63];
    char hname[33];
    char IPAddr[12];
    char GWAddr[12];
    char SNMask[12];
    uint8_t DbgLog;
    uint8_t DHCP;
  } config_t;

config_t deviceconfig; 
WiFiServer server(port);
WiFiClient serverClients[MAX_SRV_CLIENTS];
void setup() {
  // put your setup code here, to run once:
  memset(buf,0,sizeof(buf));
  memset(curStatus,0,sizeof(curStatus));
  Serial.begin(9600);
  Serial.print("\n");
  Serial.println("legaCy RGB Strip WiFi Controller v: 0.1");
  Serial.println("Starting Controller");
  pinMode(D1,OUTPUT);
  pinMode(D2,OUTPUT);
  pinMode(D3,OUTPUT);
  pinMode(D5,INPUT_PULLUP);
  analogWriteFreq(20000);
  if(digitalRead(D4) == LOW){
    doSerialSetup();
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print('\n');
  Serial.println("Connected, now listening on port 1337");
  server.begin();
  server.setNoDelay(true);
  Serial.println("Setup complete");
}

void doSerialSetup(){
  int choice = -1;
  while(digitalRead(D5) == LOW) {
    Serial.print("\n \r");
    Serial.println("--[Serial Setup]----");
    Serial.println("[1] - SET SSID");
    Serial.println("[2] - SET Password");
    Serial.println("[3] - SET IP Address");
    Serial.println("[4] - SET Gateway Address");
    Serial.println("[5] - SET Subnet Mask");
    Serial.println("[6] - ENABLE/DISABLE DHCP");
    Serial.println("[7] - SET Hostname");
    Serial.println("[8] - Show current config");
    Serial.println("[9] - Save & exit");
    Serial.println("[0] - Just exit");
  
    while(Serial.available() == 0){
      delay(100);
    }
    choice = Serial.read();
    switch(choice){
      case 1: menussid(); break;
      case 2: menupass(); break;
      case 3: menuip();   break;
      case 4: menugate(); break;
      case 5: menusubn(); break;
      case 6: menudhcp(); break;
      case 7: menuhost(); break;
      case 8: menushow(); break;
      case 9: menusave(); break;
      case 0: menuexit(); break;    
    }
  }
}

void menussid(){
  Serial.println("Enter your WiFi SSID");
  String bufSSID;
  int len = 0;
  while(Serial.available()){
    bufSSID = Serial.readString();
    len = bufSSID.length()+1;
    bufSSID.toCharArray(deviceconfig.ssid,len);
  }
  Serial.println("WiFi SSID set!");  
}
void menupass(){
  Serial.println("Enter your WiFi Password");
  String bufPASS;
  int len = 0;
  while(Serial.available()){
    bufPASS = Serial.readString();
    len = bufPASS.length()+1;
    bufPASS.toCharArray(deviceconfig.password,len);
  }
  Serial.println("WiFi password set!");  
}
void menuip(){
  Serial.println("Not Implemented");
  
}
void menugate(){
 Serial.println("Not Implemented");
}
void menusubn(){
  Serial.println("Not Implemented");
}
void menudhcp(){
  Serial.println("Not Implemented");
}
void menuhost(){
  Serial.println("Not Implemented");
}
void menushow(){
  Serial.println("Not Implemented");
}
void menusave(){
  Serial.println("Not Implemented");
}
void menuexit(){
  Serial.println("Not Implemented");
}


void setLeds(){
  int r,g,b;
  r = map(curStatus[0],0,254,0,1023);
  g = map(curStatus[1],0,254,0,1023);
  b = map(curStatus[2],0,254,0,1023);
  analogWrite(D1,r);
  analogWrite(D2,g);
  analogWrite(D3,b);
  Serial.println("--[Setting LEDs]");
  Serial.print("R=");
  Serial.print(r,DEC);
  Serial.print(" G=");
  Serial.print(g,DEC);
  Serial.print(" B=");
  Serial.print(b,DEC);
  Serial.println("----");
}

void fadePR(){
  if (curStatus[0] <= 254){
    curStatus[0]++;
    setLeds();
  }
  else
    fadeStep++;  
}

void fadePG(){
if (curStatus[1] < 254){
    curStatus[1]++;
    setLeds();
  }
  else
    fadeStep++;  
}

void fadePB(){
if (curStatus[2] < 254){
    curStatus[2]++;
    setLeds();
  }
  else
    fadeStep++;  
}

void fadeMR(){
  if (curStatus[0] > 0){
    curStatus[0]--;
    setLeds();
  }
  else
    fadeStep++;    
}

void fadeMG(){
  if (curStatus[1] > 0){
    curStatus[1]--;
    setLeds();
  }
  else
    fadeStep++;  
}

void fadeMB(){
  if (curStatus[2] > 0){
    curStatus[2]--;
    setLeds();
  }
  else
    fadeStep = 0;  
}


void fade(){
  onOff = 1;
  switch(fadeStep){
    case 0:fadePR();break;
    case 1:fadePG();break;
    case 2:fadePB();break;
    case 3:fadeMR();break;
    case 4:fadeMG();break;
    case 5:fadeMB();break;
  }
  delay(buf[4]);
}

void strobe(){
  onOff = 1;
  if(abs(millis() -lastStrobe) > (buf[4]*10)){
    if (curStatus[0] == 0){
      curStatus[0] = 254;
      curStatus[1] = 254;
      curStatus[2] = 254;
      setLeds();
    }
    else{
      curStatus[0] = 0;
      curStatus[1] = 0;
      curStatus[2] = 0;
      setLeds();
    }  
    lastStrobe = millis();
 } 
}

void loop() {
  int i;
  //check if there are any new clients
  if (server.hasClient()) {
    //find free/disconnected spot
    
    for (i = 0; i < MAX_SRV_CLIENTS; i++)
      if (!serverClients[i]) { // equivalent to !serverClients[i].connected()
        serverClients[i] = server.available();
        Serial.println("New client connected");
        break;
      }
  }
  for (i = 0; i < MAX_SRV_CLIENTS; i++){
    if (serverClients[i].available() >= 5){      
      serverClients[i].read(buf,sizeof(buf));
      Serial.println("--[Packet received]");
      Serial.print(buf[0],HEX);
      Serial.print(" ");
      Serial.print(buf[1],HEX);
      Serial.print(" ");
      Serial.print(buf[2],HEX);
      Serial.print(" ");
      Serial.print(buf[3],HEX);
      Serial.print(" ");
      Serial.print(buf[4],HEX); 
      Serial.println("----");
      newPacket = 1;
    }
  }
 int operation = buf[0];
 switch(operation){
  case 0: if (onOff == 1){onOff = 0; curStatus[0] = 0;curStatus[1] = 0;curStatus[2] = 0; if(newPacket == 1){setLeds();newPacket = 0;}} break;//Off
  case 1: onOff = 1 ;curStatus[0] = buf[1]; curStatus[1]=buf[2];curStatus[2] = buf[3]; if(newPacket == 1){setLeds();newPacket = 0;}break;//Static RGB Color
  case 2: fade();break;//Fade RGB Colors
  case 3: strobe();break;//Strobe
 }
}

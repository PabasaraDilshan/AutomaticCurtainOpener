#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <WebSocketsClient.h>

#include <Stepper.h>
#define USE_SERIAL Serial
IPAddress local_IP(192, 168, 8, 1);
// Set your Gateway IP address
IPAddress gateway(192, 168, 8, 1);
IPAddress subnet(255, 255, 0, 0);
//IPAddress primaryDNS(8, 8, 8, 8);   //optional
//IPAddress secondaryDNS(8, 8, 4, 4);
ESP8266WebServer server;
Stepper myStepper(200, 12, 13, 5, 16);

int stepCount = 0;
String serverURL;
WebSocketsClient webSocket;
uint8_t pin_led = 2;

char* ssid = "Dialog 4G 162";
char* password = "O5nnCDdM";
char* APssid = "Curtain";
char* APpassword = "12345678";

char webpage[] PROGMEM = R"=====(
<html>
<head>
<script>
var Socket;
function init(){
  Socket = new WebSocket('ws://'+window.location.hostname+':81/');
  Socket.onmessage = (event)=>{
    console.log(event);
    }
  
  }

</script>
</head>
<body onload="javascript:init()">
<form action="/toggle">
<button> TOGGLE </button>
</form>

</body>
</html>
)=====";

char connectPage[] PROGMEM = R"=====(
<html>
<head>
<script>
var Socket;
function init(){
  Socket = new WebSocket('ws://'+window.location.hostname+':81/');
  Socket.onmessage = (event)=>{
    console.log(event);
    }
  
  }

</script>
</head>
<body onload="javascript:init()">
<form action="/wlan"  method="post">

<input type="text" name = "ssid">
<input type="text" name = "pw">
 <input type="submit" value="Change Wifi">
</form>
<form action="/change-server"  method="post">

<input type="text" name = "serverip">
 <input type="submit" value="Change Server">
</form>
</body>
</html>
)=====";


int m = 0;// not calibrated
int width = -1;
int currentPose = -1;

bool openDirection = +1;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
   String strPay="";
   int i=0;
  switch(type) {
    case WStype_DISCONNECTED:
      USE_SERIAL.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED: 
      USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);

      // send message to server when Connected
      webSocket.sendTXT("{\"type\":\"connect\", \"isCurtain\":true,\"curtainId\":11223}");
    
      break;
    case WStype_TEXT:{
      USE_SERIAL.printf("[WSc] get text: %s\n", payload);

      while(payload[i]){
        strPay += (char)payload[i];
        i++;
        
      }
      String resp;
       resp =  msgHandler(strPay);
      // send message to server
      webSocket.sendTXT(resp);}
      break;
    case WStype_BIN:
      USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
      hexdump(payload, length);

      // send data to server
//      webSocket.sendBIN(payload, length);
      break;
    case WStype_PING:
    // pong will be send automatically
            USE_SERIAL.printf("[WSc] get ping\n");
            break;
    case WStype_PONG:
            // answer to a ping we send
            USE_SERIAL.printf("[WSc] get pong\n");
            break;
    }

}

String msgHandler(String msg){

   DynamicJsonDocument doc(1024);

  // You can use a String as your JSON input.
  // WARNING: the string in the input  will be duplicated in the JsonDocument.
    deserializeJson(doc, msg);
    const size_t CAPACITY = JSON_OBJECT_SIZE(10);
    StaticJsonDocument<CAPACITY> resdoc;
    JsonObject res = resdoc.to<JsonObject>();
    JsonObject obj = doc.as<JsonObject>();
//  char* type = obj["type"];
//  const char* typeStr = obj["type"];
//  int type = str2Int(typeStr);
//  const char* data;
Serial.println(msg);
  int tp = obj["type"];
  int steps;
  int spd;
  res["case"] = tp;
  switch(tp){
    case 0:// close window
          res["type"] = "from-curtain";
          res["success"] = true; 
          res["curtainId"] = obj["curtainId"]; 
          res["username"] = obj["username"];
          res["curtainState"] = "closed";
          if(obj["speed"]){
            rotateUntilEnd(-1*openDirection,obj["speed"]);  
          }else{
            rotateUntilEnd(-1*openDirection,30);  
          }
          
        Serial.printf("Rotate some degree to neg dir \n");

     break;
    case 1:// open window
          res["type"] = "from-curtain";
          res["success"] = true; 
          res["curtainId"] = obj["curtainId"]; 
          res["username"] = obj["username"];
          res["curtainState"] = "opened";
           if(obj["speed"]){
            rotateUntilEnd(openDirection,obj["speed"]);  
          }else{
            rotateUntilEnd(openDirection,30);  
          }
          Serial.printf("Rotate some degree to positive dir \n");
      break;
    default:
      Serial.println("Nothing");
      break;
  }
  String out;
  serializeJson(res, out);
  
 Serial.printf("Sent: %s",out);
  return out;
  
}
String chars2Str(char* chars){
  int i = 0;
  String str = "";
  while(chars[i]){
        str += chars[i];
        i++;
        
      }
}
int str2Int(const char* str){
  int i = 0;
  int out = 0;
  while (str[i]){
    out = out*10 + ((int)str[i]-(int)'0');
  }
  return out;
  
  }


void blinkLED(){
// rotateStepper(4);

  digitalWrite(pin_led,!digitalRead(pin_led));
  
 }
void rotateUntilEnd(int dir,int spd){
      myStepper.setSpeed(spd);
  // step 1/100 of a revolution:
  if(dir>=0){
     while(true){
      if(digitalRead(4)==0){
        myStepper.step(-40);
        break;
      }
    myStepper.step(20);
    yield();
  }
  
  }else{
     while(true){
      if(digitalRead(4)==0){
        myStepper.step(40);
        break;
      }
    myStepper.step(-20);
    yield();
  }
  
  }
  
}
void rotateStepper(int steps,int spd){
    myStepper.setSpeed(spd);
  // step 1/100 of a revolution:
  if(steps>=0){
     for(int i =0;i<steps;i++){
      if(digitalRead(4)==0){
        myStepper.step(-40);
        break;
      }
    myStepper.step(20);
    yield();
  }
  
  }else{
     for(int i =0;i<-steps;i++){
      if(digitalRead(4)==0){
        myStepper.step(40);
        break;
      }
    myStepper.step(-20);
    yield();
  }
  
  }

  }

void toggleLED()
{
 rotateUntilEnd(1,30);
  digitalWrite(pin_led,!digitalRead(pin_led));
  server.send_P(200,"text/html", webpage);
}
void conPage(){
  server.send_P(200,"text/html", connectPage);
}

void onWlan(){
        if (server.hasArg("plain")== false){ //Check if body received

            server.send(200, "text/plain", "Body not received");
            return;

      }
      String data =  server.arg("plain");
      int ind = 0;
      int d = 0;
      String cred[] = {"",""};
      while(ind<data.length()){
        if(data[ind]=='='){
          int s = ind+1;
          while(data[ind]!='&' && ind!=data.length()){
            ind++;
          }
          cred[d] = data.substring(s,ind);
          d++;
        }
        ind++;
      }

//      Serial.println(cred[0]);
//      Serial.println(cred[1]);
      File netConf = SPIFFS.open("/conf.txt","w");
      netConf.print(cred[0]);
      netConf.print('\n');
      netConf.print(cred[1]);
      netConf.close();


      String message = "Body received:\n";
             message +=data;
             message += "\n";
      
      server.send(200, "text/plain", message);
      Serial.println(message);
}

void changeServer(){
    if (server.hasArg("plain")== false){ //Check if body received

            server.send(200, "text/plain", "Body not received");
            return;

      }
      String data =  server.arg("plain");
      int ind = 0;
      int d = 0;
      String serverUrl;
      while(ind<data.length()){
        if(data[ind]=='='){
          int s = ind+1;
          while(data[ind]!='&' && ind!=data.length()){
            ind++;
          }
          serverUrl = data.substring(s,ind);
        }
        ind++;
      }
      Serial.println("Testing2");
      Serial.println(serverUrl);
//      Serial.println(cred[0]);
//      Serial.println(cred[1]);
      File serverFile = SPIFFS.open("/server.txt","w");
      serverFile.print(serverUrl);

      serverFile.close();


      String message = "Body received:\n";
             message +=data;
             message += "\n";
      
      server.send(200, "text/plain", message);
      Serial.println(message);
  
  
  }
void calibrate(){
  while(digitalRead(4)){
    rotateStepper(1,30);
  }
   rotateStepper(-4,30);
  int len = 0;
  while(digitalRead(4)){
    rotateStepper(-1,30);
    len++;
  }
  Serial.printf("Calibrated %d\n",len);
  
}

int cloop=0;
void IntCallback(){
 Serial.print("Stamp(ms): ");
 Serial.println(millis());
}
void setup()
{
  
   Serial.begin(9600);
   pinMode(4,INPUT_PULLUP);
  pinMode(pin_led, OUTPUT);
//  rotateStepper(1,30);
  bool isFs = SPIFFS.begin();
  if(isFs){
    Serial.println("FS success");
  }else{
    Serial.println("FS failed");
    return;
  }
  File netConf = SPIFFS.open("/conf.txt","r");
  File serverFile = SPIFFS.open("/server.txt","r");
//  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");
//  boolean res = WiFi.softAP(APssid, APpassword);
  if(!serverFile){
    serverURL = "192.168.1.129";
    
    }else{
      serverURL = "";
      int c = 0;
      while(1){
         c = serverFile.read();
         if(c==-1){
          break;
         }
         serverURL+=(char)c;
     }
     Serial.println("Testing ");
      Serial.println(serverURL);
      }
  if(!netConf){
    Serial.println("Error open");
    
  
    WiFi.softAPConfig(local_IP, gateway, subnet);
     WiFi.softAP(APssid, APpassword);
  }else{
    

    int c = 0;
    String ssid1="";
    String pw1=""; 
    while(1){
         c = netConf.read();
         if(c==(int)'\n'){
          break;
         }
         if(c=='+'){
          ssid1+=" ";
          
          }else{
            ssid1+=(char)c;
            
            }
         
     }
    while(1){
         c = netConf.read();
         if(c==-1){
          break;
         }
         pw1+=(char)c;
     } 

    Serial.println(ssid);
    Serial.println(password);
    Serial.println(ssid1);
    Serial.println(pw1);
    WiFi.begin(ssid1,pw1);
//    if (!WiFi.config(local_IP, gateway, subnet)) {
//    Serial.println("STA Failed to configure");
//  }

  
  int count = 0;
  while(WiFi.status()!=WL_CONNECTED)
  {
    Serial.print(".");
    count++;
    delay(500);
    if(count>50){
      WiFi.disconnect();
      Serial.println("Failed to connec wifi");
      Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");
      boolean res = WiFi.softAP(APssid, APpassword);
      if(res){
      Serial.print("Hotspot Ready");
      }else{
        Serial.print("Hotspot not Ready");
        
        }
        break;
    }
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
    
  }

  
  netConf.close();
  
  delay(1000);

  server.on("/",conPage);
  server.on("/toggle",toggleLED);
  server.on("/wlan",onWlan);
  server.on("/change-server",changeServer);
  server.begin();
   delay(1000);
   Serial.print("Connecting to websocket");
   Serial.print(serverURL);
  webSocket.begin(serverURL,80,"/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
   cloop= 0;
}

void loop()
{
//  if(cloop%10){
//      Serial.printf("Read %d %d\n",cloop, digitalRead(4));
//  }

  webSocket.loop();

  
  server.handleClient();
//  cloop++;
}

/*------------------------------------------------------------------------------
  07/01/2018
  Author: Makerbro
  Platforms: ESP8266
  Language: C++/Arduino
  File: webserver_html.ino
  ------------------------------------------------------------------------------
  Description: 
  Code for YouTube video demonstrating how to use HTML weppages in a web 
  server's response.
  https://youtu.be/VNgFbQAVboA

  Do you like my videos? You can support the channel:
  https://patreon.com/acrobotic
  https://paypal.me/acrobotic
  ------------------------------------------------------------------------------
  Please consider buying products from ACROBOTIC to help fund future
  Open-Source projects like this! We'll always put our best effort in every
  project, and release all our design files and code for you to use. 

  https://acrobotic.com/
  https://amazon.com/acrobotic
  ------------------------------------------------------------------------------
  License:
  Please see attached LICENSE.txt file for details.
------------------------------------------------------------------------------*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <FS.h>
IPAddress local_IP(192, 168, 1, 184);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4);
ESP8266WebServer server;
WebSocketsServer webSocket = WebSocketsServer(81);
uint8_t pin_led = 2;
char* ssid = "SLT-ADSL-60AE3";
char* password = "MH2236311";
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
 <input type="submit" value="Submit">
</form>

</body>
</html>
)=====";

void setup()
{
   Serial.begin(9600);
  pinMode(pin_led, OUTPUT);
  bool isFs = SPIFFS.begin();
  if(isFs){
    Serial.println("FS success");
  }else{
    Serial.println("FS failed");
    return;
  }
  File netConf = SPIFFS.open("/conf.txt","r");
  if(!netConf){
    Serial.println("Error open");
    
    WiFi.softAP(APssid, APpassword);
    WiFi.softAPConfig(local_IP, gateway, subnet);
  }else{
    

    int c = 0;
    String ssid1="";
    String pw1=""; 
    while(1){
         c = netConf.read();
         if(c==(int)'\n'){
          break;
         }
         ssid1+=(char)c;
     }
    while(1){
         c = netConf.read();
         if(c==-1){
          break;
         }
         pw1+=(char)c;
     } 

    Serial.println(ssid1);
    Serial.println(pw1);
    WiFi.begin(ssid,password);
    
  }
 
  
  netConf.close();
  
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  
 
  while(WiFi.status()!=WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/",conPage);
  server.on("/toggle",toggleLED);
  server.on("/wlan",onConnect);
  server.begin();
  webSocket.begin();
}

void loop()
{
  webSocket.loop();
  server.handleClient();
  if(Serial.available()>0){
    char c[] = {(char)Serial.read()};
    webSocket.broadcastTXT(c,sizeof(c));
  }
}

void toggleLED()
{
  digitalWrite(pin_led,!digitalRead(pin_led));
  server.send_P(200,"text/html", webpage);
}
void conPage(){
  server.send_P(200,"text/html", connectPage);
}

void onConnect(){
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

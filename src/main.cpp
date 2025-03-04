#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

void moveStepperAngle(int steps, int direction);
void handleMove30();

int Pin1 = 13; //IN1 is connected to 13 
int Pin2 = 12; //IN2 is connected to 12  
int Pin3 = 14; //IN3 is connected to 14 
int Pin4 = 27; //IN4 is connected to 27 

int pole1[] = {0,0,0,0, 0,1,1,1, 0}; //pole1, 8 step values
int pole2[] = {0,0,0,1, 1,1,0,0, 0}; //pole2, 8 step values
int pole3[] = {0,1,1,1, 0,0,0,0, 0}; //pole3, 8 step values
int pole4[] = {1,1,0,0, 0,0,0,1, 0}; //pole4, 8 step values

int poleStep = 0; 
int dirStatus = 3; // stores direction status 3= stop (do not change)

String buttonTitle1[] = {"CCW", "CW", "START"};
String buttonTitle2[] = {"CCW", "CW", "START"};
String argId[] = {"ccw", "cw", "start"};

const char *ssid = "ChickenTest";
const char *password = "12345678";

WebServer server(80);

void handleRoot() {
   String HTML = "<!DOCTYPE html>"
   "<html>"
   "<head>"
   "<title>Chicken Feed Software</title>"
   "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
   "<style>"
     "html,body{"
       "width:100%;"
       "height:100%;"
       "margin:0;"
     "}"
     "*{box-sizing:border-box}"
     ".colorAll{"
       "background-color:#90ee90;"
     "}"
     ".colorBtn{"
       "background-color:#add8e6;"
     "}"
     ".angleButton,a{"
       "font-size:30px;"
       "border:1px solid #ccc;"
       "display:table-caption;"
       "padding:7px 10px;"
       "text-decoration:none;"
       "cursor:pointer;"
       "padding:5px 6px 7px 10px;"
     "}"
     "a{"
       "display:block;"
     "}"
     ".btn{"
       "margin:5px;"
       "border:none;"
       "display:inline-block;"
       "vertical-align:middle;"
       "text-align:center;"
       "white-space:nowrap;"
     "}"
   "</style>"
   "</head>"
   "<body>"
   "<h1>Chicken Feeder Software</h1>";
 
 if (dirStatus == 2) {
   HTML += "<h2><span style=\"background-color: #FFFF00\">Motor Running in CW</span></h2>";
 } else if (dirStatus == 1) {
   HTML += "<h2><span style=\"background-color: #FFFF00\">Motor Running in CCW</span></h2>";
 } 
 else if (dirStatus == 4){
  HTML += "<h2><span style=\"background-color: #FFFF00\">Motor Running</span></h2>";
 }
 else {
   HTML += "<h2><span style=\"background-color: #FFFF00\">Motor OFF</span></h2>";
 }
 
 if (dirStatus == 1) {
   HTML += "<div class=\"btn\"><a class=\"angleButton\" style=\"background-color:#f56464\" href=\"/motor?";
   HTML += argId[0];
   HTML += "=off\">";
   HTML += buttonTitle1[0];
 } else {
   HTML += "<div class=\"btn\">"
           "<a class=\"angleButton\" style=\"background-color:#90ee90\" href=\"/motor?";
   HTML += argId[0];
   HTML += "=on\">";
   HTML += buttonTitle2[0];
 }
 HTML += "</a>"
         "</div>";
 
 if (dirStatus == 2) {
   HTML += "<div class=\"btn\">"
           "<a class=\"angleButton\" style=\"background-color:#f56464\" href=\"/motor?";
   HTML += argId[1];
   HTML += "=off\">";
   HTML += buttonTitle1[1];
 } else {
   HTML += "<div class=\"btn\">"
           "<a class=\"angleButton\" style=\"background-color:#90ee90\" href=\"/motor?";
   HTML += argId[1];
   HTML += "=on\">";
   HTML += buttonTitle2[1];
 }
HTML += "</a>"
        "</div>";

if(dirStatus = 4){
  HTML += "<div class=\"btn\">"
           "<a class=\"angleButton\" style=\"background-color:#f56464\" href=\"/move30?";
   HTML += argId[2];
   HTML += "=off\">";
   HTML += buttonTitle1[2];
} 
else{
  HTML += "<div class=\"btn\">"
      "<a class=\"angleButton\" style=\"background-color:#90ee90\" href=\"/move30?";
  HTML += argId[2];
  HTML += "=on\">";
  HTML += buttonTitle2[2];
}
HTML += "</a>"
        "</div>";

 
  server.send(200, "text/html", HTML);  
}

void handleNotFound() {
  String message = "File Not Found\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}

void driveStepper(int c) {
  digitalWrite(Pin1, pole1[c]);  
  digitalWrite(Pin2, pole2[c]); 
  digitalWrite(Pin3, pole3[c]); 
  digitalWrite(Pin4, pole4[c]);   
}

void motorControl() {
  if (server.arg(argId[0]) == "on") {
    dirStatus = 1; // CCW 
  } else if (server.arg(argId[0]) == "off") {
    dirStatus = 3;  // motor OFF 
  } else if (server.arg(argId[1]) == "on") {
    dirStatus = 2;  // CW  
  } else if (server.arg(argId[1]) == "off") {
    dirStatus = 3;  // motor OFF
  }
  else if (server.arg(argId[2]) == "on"){
    dirStatus = 4;
    handleMove30();
  }
  else if (server.arg(argId[2]) == "off"){
    dirStatus = 3;
  }
  handleRoot();
}

void setup(void) {
  pinMode(Pin1, OUTPUT);  
  pinMode(Pin2, OUTPUT);   
  pinMode(Pin3, OUTPUT);   
  pinMode(Pin4, OUTPUT);   

  Serial.begin(115200);
  Serial.println("Robojax 28BYJ-48 Stepper Motor Control");

  // Set up as Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.print("Access Point Started\n");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  if (MDNS.begin("chickentest")) {
    Serial.println("MDNS responder started");
    Serial.println("access via http://chickentest");
  }

  server.on("/", handleRoot);
  server.on("/motor", HTTP_GET, motorControl);  
  server.on("/move30",HTTP_GET, handleMove30);         
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}


void loop(void) {
  server.handleClient();
  while(dirStatus == 1){
    if(poleStep >= 7){
      poleStep = 0;
    }
    driveStepper(poleStep);
    poleStep++; 
    delay(1);
    server.handleClient();
  }
  while(dirStatus == 2){
    poleStep--; 
    driveStepper(poleStep);
    if(poleStep == 0){
      poleStep = 7;
    }
    delay(1);
    server.handleClient();
  }
  if(dirStatus == 3){
    poleStep = 8;
  }
  delay(1);
}

void moveStepperAngle(int steps, int direction){
  for (int i = 0; i < steps; i++){
    if(direction == 1){ //CCW
      poleStep = (poleStep + 1) % 8;
    }
    else{
      poleStep = (poleStep - 1 + 8) % 8;
    }
    driveStepper(poleStep);
    delay(2);
  }
}

void handleMove30(){
  int steps = 4096; // 360 degrees = 4096 steps. Ex: 1024 = 90 deg.

  moveStepperAngle(steps,2);
  delay(2000);
  moveStepperAngle(steps,1);
  dirStatus = 3;
  handleRoot();
}
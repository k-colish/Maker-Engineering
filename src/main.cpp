#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>


int Pin1 = 13; //IN1 is connected to 13 
int Pin2 = 12; //IN2 is connected to 12  
int Pin3 = 14; //IN3 is connected to 14 
int Pin4 = 27; //IN4 is connected to 27 

int pole1[] = {0,0,0,0, 0,1,1,1, 0}; //pole1, 8 step values
int pole2[] = {0,0,0,1, 1,1,0,0, 0}; //pole2, 8 step values
int pole3[] = {0,1,1,1, 0,0,0,0, 0}; //pole3, 8 step values
int pole4[] = {1,1,0,0, 0,0,0,1, 0}; //pole4, 8 step values

bool systemOn = false;
bool infoDisplayed = false;
int poleStep = 0; 
int dirStatus = 3; // stores direction status 3= stop (do not change)
String selectedSize = "Small"; // default to small feed size

void moveStepperAngle(int steps, int direction);
void handleMove30();
void displayInfo();
void startStop();
void smallSize();
void mediumSize();
void largeSize();

String buttonTitle1[] = {"CCW", "CW", "START"};
String buttonTitle2[] = {"CCW", "CW", "START"};
String argId[] = {"ccw", "cw", "start", "info", "small", "medium", "large", "hours", "minutes"};

const char *ssid = "ChickenWIFI";
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
       "background-color:#ccc;"
     "}"
     "h1{"
      "text-align:center;"
     "}"
     "h2{"
     "text-align:center;"
     "font-size:30px;"
     "}"
     ".offButton{"
      "font-size:30px;"
      "background-color: #ccc;"
      "color:black;"
      "border:2px solid #DA1212;"
      "border-radius: 5px;"
      "transition-duration: 0.4s;"
      "cursor:pointer;"
      "padding:5px 7px;"
      "text-decoration:none;"
     "}"
     ".offButton:hover{"
     " background-color: #DA1212;"
     "}"
     ".onButton{"
      "font-size:30px;"
      "color:black;"
      "background-color: #ccc;"
      "border:2px solid #1DD419;"
      "border-radius: 5px;"
      "transition-duration: 0.4s;"
      "cursor:pointer;"
      "padding:5px 7px;"
      "text-decoration:none;"
     "}"
     ".onButton:hover{"
     " background-color: #1DD419;"
     "}"
     ".btn{"
       "margin:0px 5px;"
       "border:none;"
       "display:inline-block;"
       "vertical-align:middle;"
       "white-space:nowrap;"
     "}"
     "form{"
     "font-size:20px;"
     "text-align:center;"
     "padding:2px 0px 2px 10px;"
     "}"
     "input{"
     "padding:2px 7px 2px 4px;"
     "}"
     ".infoButton{"
      "font-size:20px;"
      "color:black;"
      "background-color: #ccc;"
      "border: 2px solid #000000;"
      "border-radius:50%;"
      "padding: 1px 10px 2px 10px;"
      "transition-duration:0.4s;"
      "text-decoration:none;"
     "}"
     ".infoButton:hover{"
      "color:white;"
      "background-color:black;"
     "}"
     "p{"
      "text-align:center;"
      "font-size:25px;"
     "}"
   "</style>"
   "</head>"
   "<body>"
    "<h1>Chicken Feeder Software</h1>"
      "<div>"
        "<p>";
          
if(infoDisplayed){  // Add info button and helpful information about what each button does
  HTML += "Hide Help: "
          "<a class=\"infoButton\" href=\"/info?";
  HTML += argId[3];
  HTML += "=hide\"><b>i</b></a>"
      "</div>";
  HTML += "<p>"
  "<b>Feed Size</b>: Each size determines how long the feed will be allowed to fall out of the feeder. (Selected size is shown as red)<br>The larger the feed, the more time for it to fall out.<br>"
  "<b>Feeding Period</b>: This time is the delay between feeding times.       <br>Ex) 1 hour means it will take 1 hour from the feeder closing to the feeder opening again.";
}
else{ // Add info button without the information
  HTML += "Show Help: "
          "<a class=\"infoButton\" href=\"/info?";
  HTML += argId[3];
  HTML += "=show\"><b>i</b></a>"
      "</div>";
}
if(systemOn){ // Add Stop button with arg to turn off the system
  HTML += "<h2>System Status: On</h2>"
    "<form>"
      "<div class=\"btn\">"
        "<a class=\"offButton\" href=\"/startStop?";
  HTML += argId[2];
  HTML += "=false\">Stop</a>" // Use offButton to display colors to turn off
      "</div>"
    "</form>";
}
else{ // Add start button with arg to turn on the system
  HTML += "<h2>System Status: Off</h2>"
    "<form>"
      "<div class=\"btn\">"
        "<a class=\"onButton\" href=\"/startStop?";
  HTML += argId[2];
  HTML += "=true\">Start</a>" // Use onButton to display colors to turn on
      "</div>"
    "</form>";
}

if(selectedSize == "Small"){
  HTML += "<h2>Feed Size:</h2>"
"<form>"
  "<div class=\"btn\">"
    "<a class =\"offButton\" href=\"/smallSize?";
HTML += argId[4];
HTML += "=off\">Small</a>"
  "</div>"
  "<div class=\"btn\">"
    "<a class =\"onButton\" href=\"/mediumSize?";
HTML += argId[5];
HTML += "=on\">Medium</a>"
  "</div>"
  "<div class=\"btn\">"
    "<a class =\"onButton\" href=\"/largeSize?";
  HTML += argId[6];
  HTML += "=on\">large</a>"
  "</div>"
"</form>";
}
else if(selectedSize == "Medium"){
  HTML += "<h2>Feed Size:</h2>"
"<form>"
  "<div class=\"btn\">"
    "<a class =\"onButton\" href=\"/smallSize?";
HTML += argId[4];
HTML += "=on\">Small</a>"
  "</div>"
  "<div class=\"btn\">"
    "<a class =\"offButton\" href=\"/mediumSize?";
HTML += argId[5];
HTML += "=off\">Medium</a>"
  "</div>"
  "<div class=\"btn\">"
    "<a class =\"onButton\" href=\"/largeSize?";
  HTML += argId[6];
  HTML += "=on\">large</a>"
  "</div>"
"</form>";
}
else if(selectedSize =="Large"){
  HTML += "<h2>Feed Size:</h2>"
"<form>"
  "<div class=\"btn\">"
    "<a class =\"onButton\" href=\"/smallSize?";
HTML += argId[4];
HTML += "=on\">Small</a>"
  "</div>"
  "<div class=\"btn\">"
    "<a class =\"onButton\" href=\"/mediumSize?";
HTML += argId[5];
HTML += "=on\">Medium</a>"
  "</div>"
  "<div class=\"btn\">"
    "<a class =\"offButton\" href=\"/largeSize?";
  HTML += argId[6];
  HTML += "=off\">large</a>"
  "</div>"
"</form>";
}
HTML += "<h2>Feeding Period:</h2> "
    "<form action=\"/period?\">"
      "<label for=\"hours\">Hours:</label>"
      "<input type=\"number\" id=\"hours\" name=\"minutes\" min=\"0\" max=\"24\" placeholder=\"0-24\"";
  HTML += argId[7];
  HTML +="=>"
        
      "<label for=\"minutes\">Minutes:</label>"
      "<input type=\"number\" id=\"minutes\" name=\"minutes\" min=\"0\" max=\"59\" placeholder=\"0-59\"><br><br>"

      "<input type=\"submit\" value=\"Set Time\">"
    "</form>";
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
    dirStatus = 4; // call move30
  }
  else if (server.arg(argId[2]) == "off"){
    dirStatus = 3;
  }
  handleRoot();
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
  handleRoot();
}

void displayInfo(){
  if(server.arg(argId[3]) == "show"){
    infoDisplayed = true;
  }
  else infoDisplayed = false;
  handleRoot();
}

void startStop(){
  // If button argument is false, turn system on, setting the buttons arg to true
  if(server.arg(argId[2]) == "true"){
    systemOn = true;
  }
  else systemOn = false;
  handleRoot();
}

void smallSize(){
  if(server.arg(argId[4]) == "on"){
    selectedSize = "Small";
  }
  handleRoot();
}
void mediumSize(){
  if(server.arg(argId[5]) == "on"){
    selectedSize = "Medium";
  }
  handleRoot();
}
void largeSize(){
  if(server.arg(argId[6]) == "on"){
    selectedSize = "Large";
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
  server.on("/info", HTTP_GET, displayInfo);
  server.on("/startStop", HTTP_GET, startStop);
  server.on("/smallSize", HTTP_GET, smallSize);
  server.on("/mediumSize", HTTP_GET, mediumSize);
  server.on("/largeSize", HTTP_GET, largeSize);
  server.on("/motor", HTTP_GET, motorControl);  
  server.on("/move30", HTTP_GET, handleMove30);         
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
  while(dirStatus == 4){
    handleMove30();
    dirStatus = 3;
    server.handleClient();
  }
  if(dirStatus == 3){
    poleStep = 8;
  }
  delay(1);
}


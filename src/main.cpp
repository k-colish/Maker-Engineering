#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <AccelStepper.h>

#define STEP_PIN 13
#define DIR_PIN 12

int openDelay;
int targetPOS = 267;
int homePOS = 0;
int feedHours = 0;
int feedMinutes = 0;
int interval;
unsigned long lastFeedTime = 0;

bool systemOn = false;
bool infoDisplayed = false;
int poleStep = 0; 
int hours = 0;
int minutes = 0;
int dirStatus = 3; // stores direction status 3= stop (do not change)
String selectedSize = "Small"; // default to small feed size

void displayInfo();
void startStop();
void smallSize();
void mediumSize();
void largeSize();
void foodRelease();
void setPeriod();

String buttonTitle1[] = {"CCW", "CW", "START"};
String buttonTitle2[] = {"CCW", "CW", "START"};
String values = "";
String argId[] = {"ccw", "cw", "start", "info", "small", "medium", "large", "hours", "minutes"};

const char *ssid = "ChickenTEST";
const char *password = "12345678";

WebServer server(80);

AccelStepper stepper1(1, STEP_PIN, DIR_PIN);

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
   "</head>";
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
    "<form name=\"period\" action=\"/period?\" method=\"GET\">"
      "<label for=\"hours\">Hours:</label>"
      "<input type=\"number\" id=\"hours\" name=\"hours\" min=\"0\" max=\"24\" placeholder=\"0-24\" required>";
        
      "<label for=\"minutes\">Minutes:</label>"
      "<input type=\"number\" id=\"minutes\" name=\"minutes\" min=\"0\" max=\"59\" placeholder=\"0-59\" required><br><br>"

      "<input type=\"submit\" value=\"Set Time\">"
    "</form>";
  HTML += "<p>";
  HTML += hours;

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

void displayInfo(){
  if(server.arg(argId[3]) == "show"){
    infoDisplayed = true;
  }
  else infoDisplayed = false;
  handleRoot();
}

void startStop(){
  //If button argument is false, turn system on, setting the buttons arg to true
  if(server.arg(argId[2]) == "true"){
    systemOn = true;
    lastFeedTime = millis();
  }
  else systemOn = false;

  if(selectedSize == "Small"){
    openDelay = 500;
  }
  if(selectedSize == "Medium"){
    openDelay = 1000;
  }
  if(selectedSize == "Large"){
    openDelay = 1500;
  }
 
  handleRoot();
}

void foodRelease(){
  stepper1.moveTo(targetPOS);
  stepper1.runToPosition();
  delay(openDelay);
  stepper1.moveTo(homePOS);
  stepper1.runToPosition();
}

void setPeriod() {
  if (server.hasArg("hours") && server.hasArg("minutes")) {
    feedHours = server.arg("hours").toInt();
    feedMinutes = server.arg("minutes").toInt();
  }
  interval = (feedHours * 60 + feedMinutes) * 60 * 1000;
  Serial.print("Feeder will open every:");
  Serial.println(interval); 
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

void setPeriod(){
  values = server.arg("hours");
  hours = atoi(values.c_str());
  handleRoot();
}

void setup(void) {
  stepper1.setMaxSpeed(1000);
  stepper1.setAcceleration(1000);
  stepper1.setCurrentPosition(0);

  Serial.begin(115200);

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
  server.on("/period", HTTP_GET, setPeriod);
  server.on("/startStop", HTTP_GET, startStop);
  server.on("/smallSize", HTTP_GET, smallSize);
  server.on("/mediumSize", HTTP_GET, mediumSize);
  server.on("/largeSize", HTTP_GET, largeSize);
  server.on("/period", HTTP_GET, setPeriod);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  stepper1.run();

  if(systemOn){
    unsigned long currentTime = millis();
    if(currentTime - lastFeedTime >= interval){
      foodRelease();
      lastFeedTime = currentTime;
    }
  }

  delay(1);
}


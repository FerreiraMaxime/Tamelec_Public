// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
//temp
#include <OneWire.h>
#include <DallasTemperature.h>
//Carte SD
#include "FS.h"
#include "SD.h"
#include <SPI.h>
#include <NTPClient.h>
//transfert de fichiers
#include <WiFiUdp.h>
#include <iostream>


using namespace std;

// Data wire is connected to GPIO 4
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// Variables to store temperature values
String temperatureC = "";
String Consommation = "";


// nom reseau et mdp wifi
//#define ssid  "Linksys-SN-EC-A212"
//#define password "BtsStGab"

// Replace with your network credentials
const char* ssid = "Linksys-SN-EC-A212";
const char* password = "BtsStGab";

unsigned long lastTime = 0;  
unsigned long timerDelay = 15000;

File monFichier;

String ConsommationenKwh(){
  //faire le calcul ici.
  float Conso = 20.0;
  return String(Conso);
  // Attendre la partie à mathias ou le chercher sur internet
}
String readDSTemperatureC() {
  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  // sensors.setResolution(12);
  // analogReadResolution(12);
  // int Valeur_brute = analogRead(A4);
  // float tempC = (float) Valeur_brute*3.3/4096.0*100;
  // float tempC = -127.00;

  // LE code est bon, le problème vient du cablage
  //probleme reglee

  sensors.setResolution(12);
  sensors.requestTemperatures(); 
  float tempC = sensors.getTempCByIndex(0);

  if(tempC == -127.00) {
    Serial.println("Failed to read from DS18B20 sensor");
    Serial.println(tempC);
    return "--";
  } else {
    Serial.print("Temperature Celsius: ");
    Serial.println(tempC); 
  }
  
  return String(tempC);
  
}


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String processor(const String& var){
  Serial.println(var);
  
  if(var == "TEMPERATUREC"){
    return temperatureC;
  }
  if (var == "CONSOMMATION"){
    return Consommation; 
  }
  return temperatureC;
}

void initSDCard(){
  if(!SD.begin()){
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

void appendFile(fs::FS &fs, const char * path, const char * message) {


  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Echec de l'ouverture du fichier\n");
    SD.mkdir(path);
    return;
  }
 

  file.print("Temperature");
  file.print(";");
  file.print(temperatureC);
  file.print(";");
  file.print("Consommation");
  file.print(";");
  file.print(Consommation);
  file.print(";");

  file.close();


}

void eraseFile(fs::FS &fs, const char * path, const char * message) {

  if(SD.exists(path)) {
    if(!SD.rmdir(path)) {
          Serial.println(F("Erreur suppression dossier"));
    for(;;); // Attend appui sur bouton RESET
  }
}
  
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  sensors.begin();

  temperatureC = readDSTemperatureC();
  Consommation = ConsommationenKwh();

  //pinMode(ledPin, OUTPUT);

  //Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());



  // // Route for root / web page
  // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  //   request->send(SPIFFS, "/index.html", String(), false, processor);
  // });
  
  // // Route to load style.css file
  // server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
  //   request->send(SPIFFS, "/style.css", "text/css");
  // });


  initSDCard();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/index.html", String(), false, processor);
  });

  server.serveStatic("/", SD, "/");
  

  // Les boutons peuvent etre utile pour récup les fichiers


  server.on("/temperaturec", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "/index.html", temperatureC.c_str());
  });
    server.on("/consommationv", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "/index.html", Consommation.c_str(), processor);
  });



  // Start server
  server.begin();
}
 
void loop(){

  if ((millis() - lastTime) > timerDelay) {

    temperatureC = readDSTemperatureC();
    Consommation = ConsommationenKwh();

       // ofstream
    char message[100];
    eraseFile(SD, "/Mesures.csv", message);
    appendFile(SD, "/Mesures.csv", message);

    lastTime = millis();
 }  

}
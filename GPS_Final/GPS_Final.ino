
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#define GPSSerial Serial1
#define GPSECHO false
#include <Adafruit_GPS.h>
#include <LiquidCrystal.h>

const char SSID[]     = "##########";    // Network SSID (name)
const char PASS[]     = "##########";    // Network password

float currentLatitude;
float targetLatitude, storedLatitude;
float targetLongitude, storedLongitude;
float timeToTarget;
bool hitTargetValue = false, localHitTargetValue = false;
//Event handler function references for IOT cloud
void initProperties(){
  ArduinoCloud.addProperty(currentLatitude, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(targetLatitude, READWRITE, ON_CHANGE, onTargetLatitudeChange);
  ArduinoCloud.addProperty(targetLongitude, READWRITE, ON_CHANGE, onTargetLongitudeChange);
  ArduinoCloud.addProperty(timeToTarget, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(hitTargetValue, READ, ON_CHANGE, NULL);

}
WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);

Adafruit_GPS GPS(&GPSSerial);

//LCD pins
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int btnPin = 10;
bool btnState = false, pastbtnState = false, isConnected = false;

double longitude, latitude; 

void setup() {
  // make this baud rate fast enough to we aren't waiting on it
  Serial.begin(9600);
  Serial.println("Start!");
  // 9600 baud is the default rate for the Ultimate GPS
  GPS.begin(9600);
  GPS.sendCommand("$PGCMD,33,0*6D");
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);

  // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
  pastbtnState = digitalRead(btnPin);
  initProperties();
  //connect to the IoT cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection,false);

  setDebugMessageLevel(4);   //Get Cloud Info/errors, 0 (only errors) up to 4
  ArduinoCloud.printDebugInfo();

  lcd.print("Initializing...");
  pinMode(btnPin, INPUT);
  delay(1000);
  lcd.clear();
  localHitTargetValue = false;
  /*while (ArduinoCloud.connected() != 1) {
    ArduinoCloud.update();
    lcd.setCursor(0,0);
    lcd.print("Waiting For");
    lcd.setCursor(0,1);
    lcd.print("Connection...");
    delay(500);
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connected!");
  delay(3000);*/
}

//Note: Initial GPS readings are 0 when attempting to initialy receive location data from
//the satellite.
void loop() {
  hitTargetValue = localHitTargetValue;
  checkbtnState();
  readGPS();
  manageCloud();
  //If within a certain area and satellite is not initializing set hit target value to true. 
  if((latitude <= storedLatitude + 0.01 && latitude >= storedLatitude - 0.01) &&
     (longitude <= storedLongitude + 0.01 && longitude >= storedLongitude - 0.01) &&
     storedLongitude != 0 && storedLatitude != 0 && longitude != 0 && latitude != 0){
    localHitTargetValue = true;
  }
  delay(0);
}
void checkbtnState(){
  //button was pressed rest screen
  if(digitalRead(btnPin) && !pastbtnState){
    btnState = !btnState;
    lcd.clear();
    if(btnState){
      printCords();
    }else{
      printDistance();
    }
  }
  pastbtnState = digitalRead(btnPin);
}
void manageCloud(){
      ArduinoCloud.update();
  if(ArduinoCloud.connected() == 1){
    if(!(targetLatitude == NULL || targetLatitude == 0 || targetLongitude == NULL || targetLongitude == 0)){
    storedLatitude = targetLatitude;
    storedLongitude = targetLongitude;
    }

    isConnected = true;
    Serial.println("Is connected: true");
  }else{
    ArduinoCloud.update();
    isConnected = false;    
    Serial.println("Is connected: false");
  }

}
void readGPS(){
  String NMEA1, NMEA2;
  char c;
  float degWhole, degDec;
  clearGPSBuffer();
  while(!GPS.newNMEAreceived()){//read first NMEA sentence
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA());
  NMEA1=GPS.lastNMEA();

  while(!GPS.newNMEAreceived()){//read the second NMEA sentence
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA());
  
  NMEA2=GPS.lastNMEA();
  
  //-- I commented out the global navigation satellite system decimal degress 
  //data converter code so all the location data can fit onto the LCD screen. --
  
  //Get data you want from sentences
  //Get/Calculate longitude
  //degWhole = float(int(GPS.longitude/100));
  //degDec = (GPS.longitude - degWhole*100)/60;
  longitude =   abs(GPS.longitude);
  latitude =   abs(GPS.latitude);

  /*if(GPS.lon=='W'){
    longitude=(-1)*longitude;
  }*/
  //Get/Calculate latitude
  //degWhole = float(int(GPS.latitude/100));
  //degDec = (GPS.latitude - degWhole*100)/60;
  //latitude = degWhole + degDec;
  /*if(GPS.lat=='S'){
    latitude=(-1)*latitude;
  }*/
  currentLatitude = latitude;
  if(btnState){
    printCords();
  }else{
    printDistance();
  }
  Serial.print(NMEA1);
  Serial.println(NMEA2);
}
void clearGPSBuffer(){//clear the buffer by reading the values
  char c;
  while(!GPS.newNMEAreceived()){
    c=GPS.read();
  }
  while(!GPS.newNMEAreceived()){
    c=GPS.read();
  }
  while(!GPS.newNMEAreceived()){
    c=GPS.read();
  }
  while(!GPS.newNMEAreceived()){
    c=GPS.read();
  }
}
//Print coordinates to LCD screen
void printCords(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Lat:" + String(GPS.latitude));
  lcd.setCursor(0, 1);
  lcd.print("Lon:" + String(GPS.longitude));
  Serial.println("LATTITUDE");
  printConnectionStatus();
}
void printDistance(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Lat:" + String(storedLatitude - latitude));
  //lcd.print("Test");
  lcd.setCursor(0, 1);
  //lcd.print("Hello World");
  //Serial.println("Hello World");
  lcd.print("Lon:" + String(storedLongitude - longitude));
  printConnectionStatus();
}
void printConnectionStatus(){
  lcd.setCursor(15, 0);
  if(isConnected){
    lcd.print("Y");
  }else{
    lcd.print("N");
  }
  lcd.setCursor(15, 1);
    if(localHitTargetValue){
    lcd.print("O");
  }else{
    lcd.print("X");
  }
}

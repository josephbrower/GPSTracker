// what's the name of the hardware serial port?
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#define GPSSerial Serial1
#define GPSECHO false
#include <Adafruit_GPS.h>
#include <LiquidCrystal.h>

const char SSID[]     = "WSUPSK";    // Network SSID (name)
const char PASS[]     = "*ydUQuRpLyY?rWd)E";    // Network password (use for WPA, or use as key for WEP)
void onTargetLatitudeChange();
void onTargetLongitudeChange();
float currentLatitude;
float targetLatitude;
float targetLongitude;
float timeToTarget;
bool hitTargetValue = false;
void initProperties(){
  ArduinoCloud.addProperty(currentLatitude, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(targetLatitude, READWRITE, ON_CHANGE, onTargetLatitudeChange);
  ArduinoCloud.addProperty(targetLongitude, READWRITE, ON_CHANGE, onTargetLongitudeChange);
  ArduinoCloud.addProperty(timeToTarget, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(hitTargetValue, READ, ON_CHANGE, NULL);

}
WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);

Adafruit_GPS GPS(&GPSSerial);

const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int btnPin = 10;
bool btnState = false, pastbtnState = false, isConnected = false;

float longitude, latitude;


void setup() {
  // make this baud rate fast enough to we aren't waiting on it
  Serial.begin(9600);
  Serial.println("Start!");
  // wait for hardware serial to appear
  //while (!Serial) delay(10);
  // 9600 baud is the default rate for the Ultimate GPS
  GPS.begin(9600);
  GPS.sendCommand("$PGCMD,33,0*6D");
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);

  // set up the LCD's number of columns and rows:

  pastbtnState = digitalRead(btnPin);
  initProperties();
  //connect to the IoT cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection,false);
  Serial.println("IN HERE1");
  setDebugMessageLevel(4);   //Get Cloud Info/errors , 0 (only errors) up to 4
  ArduinoCloud.printDebugInfo();
  Serial.println("IN HERE2");
  ArduinoCloud.update();
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Initializing...");
  pinMode(btnPin, INPUT);
  delay(1000);
  lcd.clear();
  while (ArduinoCloud.connected() != 1) {
    ArduinoCloud.update();
    lcd.setCursor(0,0);
    lcd.print("Waiting For");
    lcd.setCursor(0,1);
    lcd.print("Connection...");
    delay(500);
  }
  //lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connected!");
  delay(3000);
}


void loop() {
  checkbtnState();
  readGPS();
  manageCloud();
  /*if(latitude == targetLatitude && targetLatitude == latitude){
    hitTargetValue = true;
  }*/
  Serial.println(digitalRead(btnPin));
  delay(500);
}
void checkbtnState(){
  //button was pressed
  if(digitalRead(btnPin) && !pastbtnState){
    btnState = !btnState;
    //lcd.clear();
    if(btnState){
      printCords();
    }else{
      printDistance();
    }
  }
  pastbtnState = digitalRead(btnPin);
}
void manageCloud(){

  if(ArduinoCloud.connected() == 1){

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
  //Get data you want from sentences
  //Get/Calculate longitude
  degWhole = float(int(GPS.longitude/100));
  degDec = (GPS.longitude - degWhole*100)/60;
  longitude = degWhole + degDec;
  if(GPS.lon=='W'){
    longitude=(-1)*longitude;
  }
  //Get/Calculate latitude
  degWhole = float(int(GPS.latitude/100));
  degDec = (GPS.latitude - degWhole*100)/60;
  latitude = degWhole + degDec;
  if(GPS.lat=='S'){
    latitude=(-1)*latitude;
  }
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
void printCords(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Lat: " + String(latitude));
  lcd.setCursor(0, 1);
  lcd.print("Lon: " + String(longitude));
  Serial.println("LATTITUDE");
  printConnectionStatus();
}
void printDistance(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TLat: " + String(targetLatitude));
  //lcd.print("Test");
  lcd.setCursor(0, 1);
  //lcd.print("Hello World");
  //Serial.println("Hello World");
  lcd.print("TLon: " + String(targetLongitude));
  printConnectionStatus();
}
void printConnectionStatus(){
  lcd.setCursor(15, 0);
  if(isConnected){
    lcd.print("Y");
  }else{
    lcd.print("N");
  }
}
void onTargetLatitudeChange(){};
void onTargetLongitudeChange(){};

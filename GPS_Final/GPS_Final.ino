// what's the name of the hardware serial port?
#define GPSSerial Serial1
#define GPSECHO false
#include <Adafruit_GPS.h>
#include <LiquidCrystal.h>

Adafruit_GPS GPS(&GPSSerial);

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int btnPin = 10;
bool btnState = false, pastbtnState = false;

float longitude, latitude;


void setup() {
  // make this baud rate fast enough to we aren't waiting on it
  Serial.begin(115200);
  Serial.println("Start!");
  // wait for hardware serial to appear
  //while (!Serial) delay(10);
  // 9600 baud is the default rate for the Ultimate GPS
  GPS.begin(9600);
  GPS.sendCommand("$PGCMD,33,0*6D");
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Initializing...");
  pinMode(btnPin, INPUT);
  delay(1000);
  lcd.clear();
  pastbtnState = digitalRead(btnPin);
  if(btnState){
    lcd.setCursor(0, 0);
    lcd.print("Lat: " + String(latitude));
    lcd.setCursor(1, 0);
    lcd.print("Lon: " + String(longitude));
  }else{
    lcd.setCursor(0, 0);
    lcd.print("OFF");
  }
}


void loop() {
  checkbtnState();
  readGPS();
  delay(1000);
  //Serial.println(digitalRead(btnPin));
}
void checkbtnState(){
  //button was pressed
  if(digitalRead(btnPin) && !pastbtnState){
    btnState = !btnState;
    lcd.clear();
    if(btnState){
      lcd.setCursor(0, 0);
      lcd.print("Lat: " + String(latitude));
      lcd.setCursor(0, 1);
      lcd.print("Lon: " + String(longitude));
    }else{
      lcd.setCursor(0, 0);
      lcd.print("OFF");
    }
  }
  pastbtnState = digitalRead(btnPin);
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
}
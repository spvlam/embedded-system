#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "RTClib.h"
 
#define i2c_Address 0x3c
 
#define ONE_WIRE_BUS 4
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
 
enum CommandCode{
START, RECCORD, STOP, GETMIN, GETMAX, NONE
};
 
typedef struct Record {
  DateTime timez;
  float temp;
} Record;
 
Record maxRec, minRec;
 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
 
RTC_DS1307 rtc;
 
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 
DateTime currentTime;
float currentTempC;
CommandCode currentCommand;
 
void setupSH1106();
void setupRTC();
void updateData();
void displayTemp();
void displayTime();
void displayData();
CommandCode getCommand();
void handleCommand(CommandCode command);
 
void setup() {
  currentCommand = START;
  minRec.temp = 127;
  maxRec.temp = -127;
  Serial.begin(9600);
  delay(500);
  Serial.println("Hello");
  setupSH1106();
  setupRTC();
  sensors.begin();
}
 
void loop() {
  currentCommand = getCommand();
  handleCommand(currentCommand);
}
 
void setupSH1106() {
  delay(250);
  display.begin(i2c_Address, true); // Address 0x3C default
}
 
void setupRTC() {
  if (! rtc.begin()) {
    display.println("Couldn't find RTC");
    while (1) delay(10);
  }
 
  if (! rtc.isrunning()) {
    display.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(_DATE_), F(_TIME_)));
  }
}
 
void updateData() {
  currentTime = rtc.now();
  sensors.requestTemperatures();
  currentTempC = sensors.getTempCByIndex(0);
}
 
void displayTemp() {
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("Current temperature:");
  display.println();
 
  display.setTextSize(2);
  if (currentTempC != DEVICE_DISCONNECTED_C)
  {
 
    display.print(currentTempC);
    display.println(" C");
  }
  else
  {
    display.print("Null");
  }
  display.println();
}
 
void displayTime() {
  display.setTextSize(1);
  display.printf("%d/%d/%d %02d:%02d:%02d\n", currentTime.year(), currentTime.month(), currentTime.day(), currentTime.hour(), currentTime.minute(), currentTime.second());
}
 
void displayData() {
  display.clearDisplay();
  displayTemp();
  displayTime();
  display.display();
}
 
CommandCode getCommand() {
  String command = Serial.readStringUntil('\n');
  if (command.equals("START")) {
      return START;
  } else if (command.equals("STOP")) {
      return STOP;
  } else if (command.equals("GETMIN")) {
      return GETMIN; // Assuming GETMIN is defined as a command code
  } else if (command.equals("GETMAX")) {
      return GETMAX;
  }
  return currentCommand == START ? START : NONE;
}
 
 
void startRecord(){
  updateData();
  if(currentTempC >= maxRec.temp){
    maxRec.timez = currentTime;
    maxRec.temp = currentTempC;
    }
  if(currentTempC <= minRec.temp){
    minRec.timez = currentTime;
    minRec.temp = currentTempC;
}
  displayData();
  Serial.printf("%d/%d/%d %02d:%02d:%02d\n", currentTime.year(), currentTime.month(), currentTime.day(), currentTime.hour(), currentTime.minute(), currentTime.second());
  delay(1000);
}
 
void stopRecord(){
  Serial.println("Stop recording");
}
 
void showRecord(Record rec){
  Serial.println("Time:");
  Serial.printf("%d/%d/%d %02d:%02d:%02d\n", rec.timez.year(), rec.timez.month(), rec.timez.day(), rec.timez.hour(), rec.timez.minute(), rec.timez.second());  
  Serial.println("Temperature:");
  Serial.printf("%.2f\n", rec.temp);  
}
 
void getMin(){
  Serial.println("Max temprature record");
  showRecord(minRec);
  }
void getMax(){
  Serial.println("Max temprature record");
  showRecord(maxRec);
}
 
void handleCommand(CommandCode command){
  if(command == NONE)
    return;
  switch(command){
    case START:
      startRecord();
      break;
    case STOP:
      stopRecord();
      break;
    case GETMIN:
      getMin();
      break;
    case GETMAX:
      getMax();
      break;
    default:
      delay(20);
      break;
  }
}
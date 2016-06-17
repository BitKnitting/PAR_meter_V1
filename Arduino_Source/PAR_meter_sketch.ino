

#define DEBUG 1
#ifdef DEBUG
#define DEBUG_PRINT_BEGIN(x) Serial.begin(x)
#define DEBUG_PRINT(x)  Serial.print (x)
#define DEBUG_PRINTHEX(x) Serial.print(x,HEX)
#define DEBUG_PRINTF(x) Serial.print(F(x))
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTLNF(x) Serial.println(F(x))
#else
#define DEBUG_PRINT_BEGIN(x)
#define DEBUG_PRINT(x)
#define DEBUG_PRINTF(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTLNF(x)
#define DEBUG_PRINTHEX(x)
#endif
#include <Wire.h>  // Comes with Arduino IDE
// Get the LCD I2C Library here:
// https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
// Move any other LCD libraries to another folder or delete them
// See Library "Docs" folder for possible commands etc.
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
/*-----( Declare Constants )-----*/
const int analogInPin = A3;  // Analog input pin that the potentiometer is attached to
const byte CS = 10; //Chip Select for Adafruit's SD Shield on Arduino Uno
const unsigned long logging_interval_in_ms = .5 * 60 * 1000; // 15 mins = 15 * 60 sec/min * 1000 ms/sec

/*-----( Declare variables )-----*/
char g_dataLogFilename[] = "par.csv";
// set the LCD address to 0x27 for a 16 chars 2 line display
// A FEW use address 0x3F
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
RTC_DS1307 RTC;
unsigned long g_millisSinceStartedTiming;

void setup() {
  /////////////////////////////////////////////////////////////////////////////////////
  DEBUG_PRINT_BEGIN(9600);
  lcd.begin(16, 2);  // initialize the lcd for 16 chars 2 lines, turn on backlight
  DEBUG_PRINTLNF("\n************** PAR meter ***************");
  DEBUG_PRINTLNF("\n...Initializing Adafruit's SD Shield");
  pinMode(CS, OUTPUT);
  //////////////////////////////////////////////////////////////////////////////////////
  if (!SD.begin(CS)) {
    DEBUG_PRINTLNF("ERROR: SD card not found. Things to check:");
    DEBUG_PRINTLNF("* Is a card inserted?");
    DEBUG_PRINTLNF("* Is your wiring correct?");
    DEBUG_PRINTLNF("* Is the CS const set to 10?");
    return;
  }
  //////////////////////////////////////////////////////////////////////////////////////
  DEBUG_PRINTLNF("\n...Checking to make sure data logging file can be written to. If it can, initialize.");
  if (!initFile()) {
    DEBUG_PRINTF("ERROR: Could not log readings to ");
    DEBUG_PRINTLN(g_dataLogFilename);
    return;
  }
  //////////////////////////////////////////////////////////////////////////////////////
  DEBUG_PRINTLNF("\n...Initializing RTC.");
  RTC.begin();

  if (! RTC.isrunning()) {
    DEBUG_PRINTLNF("RTC battery needs to be replaced.  Using time this sketch was compiled.");
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  //////////////////////////////////////////////////////////////////////////////////////
  DEBUG_PRINTLNF("\n...Writing Header to Sensor Data Logging File.");
  writeHeader();
  //////////////////////////////////////////////////////////////////////////////////////
  DEBUG_PRINTLNF("\n...Writing first PAR reading to Data Loging File.");
  writeRow();
  //////////////////////////////////////////////////////////////////////////////////////
  g_millisSinceStartedTiming = millis();
}

void loop() {
  writeParValueToLCD();
  //////////////////////////////////////////////////////////////////////////////////////
  delay(1000);
  //////////////////////////////////////////////////////////////////////////////////////
  if (millis() - g_millisSinceStartedTiming > logging_interval_in_ms ) {
    writeRow();
    g_millisSinceStartedTiming = millis();
  }
}
/******************************************************************************
 * Initialize the file for logging sensor data.  Let the caller know if the
 * file cannot be opened for write.
 *******************************************************************************/
bool initFile()
{
  if (SD.exists(g_dataLogFilename)) {
    SD.remove(g_dataLogFilename);
  }
  // Check to see if the file can be opened for writing.
  File myDataFile = SD.open(g_dataLogFilename, FILE_WRITE);
  if (myDataFile) {
    myDataFile.close();
    return true;
  } else {
    return false;
  }
}
/******************************************************************************
 * Write out the names of the columns within the CSV file that is capturing PAR readings.
 *******************************************************************************/
void writeHeader() {
  File myDataFile = SD.open(g_dataLogFilename, FILE_WRITE);
  if (myDataFile) {
    myDataFile.println("Date,Time,PAR");
    myDataFile.close();
  }
}

/******************************************************************************
 * Write the Data/Time and PAR value just captured to the CSV file.
 *******************************************************************************/
void writeRow() {
  ///////////////////////////////////////////////////////////////////////////////
  char dateAndTimeStr[20];
  dateAndTimeStr[0] = 0;
  getDateTime(dateAndTimeStr);
  float parValue = getParValue();
  File myDataFile = SD.open(g_dataLogFilename, FILE_WRITE);
  myDataFile.print(dateAndTimeStr);
  DEBUG_PRINT(dateAndTimeStr);
  myDataFile.print(",");
  DEBUG_PRINTF(",");
  myDataFile.print(parValue);
  DEBUG_PRINT(parValue);
  DEBUG_PRINTLNF("");
  myDataFile.println("");
  myDataFile.close();

}
/******************************************************************************
 * Read the PAR value from the analog pin associated with the apogee PAR sensor.
 *******************************************************************************/
float getParValue() {
  return analogRead(analogInPin) / 2;
}
/******************************************************************************
 * Write the PAR value to the LCD.
 *******************************************************************************/
void writeParValueToLCD() {
  // write the value to the LCD
  float parValue = getParValue();
  lcd.setCursor(0, 0);
  lcd.print("PAR value:");
  lcd.setCursor(0, 1);
  lcd.print(parValue);
}
/******************************************************************************
 * Write the Data and Time to the CSV file.
 *******************************************************************************/
void getDateTime(char *dateAndTimeStr)
{
  DateTime now;

  // fetch the time
  now = RTC.now();
  // log date
  sprintf( dateAndTimeStr, "%02hhu/%02hhu/%02hhu,%02hhu:%02hhu:%02hhu", now.month(), now.day(), now.year(), now.hour(), now.minute(), now.second() );
}


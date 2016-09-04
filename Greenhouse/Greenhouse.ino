#include <TimeLib.h>

#include <ARTF_SDCard.h>
#include <ARTF_RTC.h>
#include <LowPower.h>

// ARTF SDCard Dependency
#include <SdFat.h>
#include <String.h>
#include <dht.h>

// ARTF RTC Dependency
#include <SPI.h>
#include <Time.h>

#define MOISTURE_PIN_ONE            A3
#define MOISTURE_PIN_TWO            A4
#define DHT_PIN                     A5
#define SUNLIGHT_PIN                A0

//Clock chip select pin
#define RTC_CS_PIN                  9

//SD card chip select pin and data file
#define SD_CS_PIN                   10
#define LOG_FILENAME                "datalog.csv"

#define RELAY_VALVE_ONE             A1
#define RELAY_VALVE_TWO             A2
#define RELAY_FAN                   3
#define RELAY_LED                   2

#define LED_CHECK_MIN            60        //takes in minutes 

//minimum moisture sensor readings percentage, its related relay will turn on whene min value meets or reduces
#define MIN_MOISTURE_ONE_PERCENTAGE               50
//minimum moisture sensor readings percentage, its related relay will turn on whene min value meets or reduces
#define MAX_MOISTURE_ONE_PERCENTAGE               60

#define MIN_MOISTURE_TWO_PERCENTAGE               70
//minimum moisture sensor readings percentage, its related relay will turn on whene min value meets or reduces
#define MAX_MOISTURE_TWO_PERCENTAGE               80


//minimum sunlight sensor readings percentage, its related relay will turn on whene min value meets or reduces
#define MIN_SUNLIGHT_PERCENTAGE               7
//miximu humidity sensor readings percentage, its related relay will turn on whene its value meets or exceeds
#define MAX_HUMIDITY_PERCENTAGE               40
//max Temperature sensor readings in celcius, its related relay will turn on whene its value meets or exceeds
#define MAX_TEMPERATURE                       28

#define TAKE_AVERAGE_OF_X_READINGS            5
#define SENSOR_READING_DELAY                  1500   //provide a value in mili seconds

//Delay in milli seconds between  each loop.
#define READING_DELAY               1000

//Nokia Screen
#include "U8glib.h"
#define SCREEN_RST                  6
#define SCREEN_CE                   7
#define SCREEN_DC                   5
#define SCREEN_DIN                  4
#define SCREEN_CLK                  8
//#define SCREEN_BL                 2



//Settings
#define SLEEP_CYCLES               97  //the board will sleep for 1 hour


//Global Variables
int sleepCount = 0;
int totalReadings = 0;

//RTC initalization
ARTF_RTC rtc(RTC_CS_PIN);

//SD card initalization
ARTF_SDCard sd(SD_CS_PIN);

//DHT initalization
dht DHT;

//Nokia Screen Initialization
U8GLIB_PCD8544 display(SCREEN_CLK, SCREEN_DIN, SCREEN_CE, SCREEN_DC, SCREEN_RST);

//global variable, it stores the last reading time
time_t lastTime;

void setup()
{
  Serial.begin(9600);

  //SD Card Chip Select PIN to output and HIGH
  pinMode(SD_CS_PIN, OUTPUT);
  digitalWrite(SD_CS_PIN, HIGH);


  //Setting Relays PINS to output
  pinMode(RELAY_VALVE_ONE, OUTPUT);
  pinMode(RELAY_VALVE_TWO, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_LED, OUTPUT);


  //Setting Relays off
  digitalWrite(RELAY_VALVE_ONE, LOW);
  digitalWrite(RELAY_VALVE_TWO, LOW);
  digitalWrite(RELAY_FAN, LOW);
  digitalWrite(RELAY_LED, LOW);



  rtc.begin();
  //rtc.setDateTime(9, 3, 2016 - 1970, 11, 58, 30); //day,month,year - 1970,hour,minute,second
  String Date = rtc.readDateAsText();
  String Time = rtc.readTimeAsText();
  time_t unixTime = rtc.readDateTime();

  lastTime = rtc.readDateTime();//minute(unixTime);

  int chk = DHT.read22(DHT_PIN);

  //digitalWrite(RELAY_LED, LOW); //will turn of LED light so the sunlight can take accurate reading.

  long moisture_one   = takeMoistureOneReading();
  long moisture_two   = takeMoistureTwoReading();
  long sunlight       = takeSunlightReading();
  long temperature    = takeTemperatureReading();
  long humidity       = takeHumidityReading();



  //turn on / off solenade VALVE one if the min percentage meets
  if (moisture_one <= MIN_MOISTURE_ONE_PERCENTAGE) {
    digitalWrite(RELAY_VALVE_ONE, HIGH);
  } else if (moisture_one >= MAX_MOISTURE_ONE_PERCENTAGE) {
    digitalWrite(RELAY_VALVE_ONE, LOW);
  }

  //turn on / off solenade VALVE two if the min percentage meets
  if (moisture_two <= MIN_MOISTURE_TWO_PERCENTAGE) {
    digitalWrite(RELAY_VALVE_TWO, HIGH);
  } else if (moisture_two >= MAX_MOISTURE_TWO_PERCENTAGE) {
    digitalWrite(RELAY_VALVE_TWO, LOW);
  }

  //turn on / off LED lights if the min percentage meets
  if (sunlight <= MIN_SUNLIGHT_PERCENTAGE) {
    digitalWrite(RELAY_LED, HIGH);
  } else {
    digitalWrite(RELAY_LED, LOW);
  }

  display.firstPage();
  do {
    draw(moisture_one, moisture_two, sunlight, temperature, humidity, Time);
  } while ( display.nextPage() );

  //SD Card begin
  sd.begin();
  sd.writeFile(LOG_FILENAME, String("Date")  + "," + String("Time") + "," + String("Unix Time") + "," + String("Sun Light") + "," + String("Moisture One") + "," +  String("Moisture Two") + "," + String("Humidity") + "," + String("Temprature"));
  Serial.print("Date\t\t\tTime\tUnix Time\tSun Light\tMoisture One\tMoisture Two\tHumidity\tTemprature\n");
  Serial.println(String(Date) + "\t\t" + Time + "\t\t" + unixTime + "\t\t" + sunlight + "\t\t" + moisture_one + "\t\t" + moisture_two + "\t\t" + + humidity + "\t\t" + temperature);

}



void loop()
{

  for (int i = 0; i  < SLEEP_CYCLES; i ++)
  {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }

  //DHT initialization, it is important each time it loops
  int chk = DHT.read22(DHT_PIN);

  //initilizing and take date/time from rtc
  rtc.begin();

  String Date = rtc.readDateAsText();


  time_t currentTime = rtc.readDateTime();

  String Time = String(hourFormat12(currentTime)) + ":" + minute(currentTime) ; //converts 24 hours format to 12 hours

  Serial.print(Time);

  long moisture_one   = takeMoistureOneReading();
  long moisture_two   = takeMoistureTwoReading();
  long sunlight;
  long temperature    = takeTemperatureReading();
  long humidity       = takeHumidityReading();

  //turn on / off solenade VALVE one if the min percentage meets
  if (moisture_one <= MIN_MOISTURE_ONE_PERCENTAGE) {
    digitalWrite(RELAY_VALVE_ONE, HIGH);
  } else if (moisture_one >= MAX_MOISTURE_ONE_PERCENTAGE) {
    digitalWrite(RELAY_VALVE_ONE, LOW);
  }

  //turn on / off solenade VALVE two if the min percentage meets
  if (moisture_two <= MIN_MOISTURE_TWO_PERCENTAGE) {
    digitalWrite(RELAY_VALVE_TWO, HIGH);
  } else if (moisture_two >= MAX_MOISTURE_TWO_PERCENTAGE) {
    digitalWrite(RELAY_VALVE_TWO, LOW);
  }



  if (minute(currentTime - lastTime) >= LED_CHECK_MIN)
  {
    lastTime =  rtc.readDateTime();  //this will reset the last time to current time so we can calculate for next 60 mins

    digitalWrite(RELAY_LED, LOW); //will turn of LED light so the sunlight can take accurate reading.
    sunlight       = takeSunlightReading();

    //turn on / off LED lights if the min percentage meets
    if (sunlight <= MIN_SUNLIGHT_PERCENTAGE) {
      digitalWrite(RELAY_LED, HIGH);
    } else  {
      digitalWrite(RELAY_LED, LOW);
    }
  }

  display.firstPage();
  do {
    draw(moisture_one, moisture_two, sunlight, temperature, humidity, Time);
  } while ( display.nextPage() );

  sd.begin();

  if (!sd.writeFile(LOG_FILENAME, String(Date) + "," + String(Time) + "," + String(currentTime) + "," + String(sunlight) + "," + String(moisture_one) + "," +  String(moisture_two) + "," + String(humidity) + "," + String(temperature))) {
    Serial.print("SD Card not available\n");
    Serial.println(String(Date) + "\t\t" + Time + "\t\t" + currentTime + "\t\t" + sunlight + "\t\t" + moisture_one + "\t\t" + moisture_two + "\t\t" + + humidity + "\t\t" + temperature);
  } else {
    //Serial.print("Sd card available\n");
    Serial.println(String(Date) + "\t\t" + Time + "\t\t" + currentTime + "\t\t" + sunlight + "\t\t" + moisture_one + "\t\t" + moisture_two + "\t\t" + + humidity + "\t\t" + temperature);
  }

  delay(READING_DELAY);
}



//Take average of moisture sensor x reading
long takeMoistureOneReading()
{
  long sumMoisture = 0;
  long avgMoisture = 0;

  for (int i = 0; i < TAKE_AVERAGE_OF_X_READINGS; i++)
  {
    sumMoisture += analogRead(MOISTURE_PIN_ONE);
    delay(SENSOR_READING_DELAY);
  }

  //delay(500);

  avgMoisture = (sumMoisture * 100 / 600) / TAKE_AVERAGE_OF_X_READINGS;
  return avgMoisture;
}

//Take average of moisture sensor x reading
long takeMoistureTwoReading()
{
  long sumMoisture = 0;
  long avgMoisture = 0;

  for (int i = 0; i < TAKE_AVERAGE_OF_X_READINGS; i++)
  {
    sumMoisture += analogRead(MOISTURE_PIN_TWO);
    delay(SENSOR_READING_DELAY);
  }

  //delay(500);

  avgMoisture = (sumMoisture * 100 / 850) / TAKE_AVERAGE_OF_X_READINGS;
  return avgMoisture;
}

//Average Sunlight of x readings
long takeSunlightReading() {

  int sumSunLight = 0;
  int avgSunLight = 0;

  for (int i = 0; i < TAKE_AVERAGE_OF_X_READINGS; i++)
  {
    sumSunLight += analogRead(SUNLIGHT_PIN) * 100 / 1000;
    delay(SENSOR_READING_DELAY);
  }


  //delay(500);

  avgSunLight = sumSunLight / TAKE_AVERAGE_OF_X_READINGS;
  return avgSunLight;
}

//Average humidity of x readings
long takeHumidityReading() {
  int sumHumidity = 0;
  int avgHumidity = 0;

  for (int i = 0; i < TAKE_AVERAGE_OF_X_READINGS; i++)
  {
    avgHumidity += DHT.humidity;
    delay(2200);  //DHT22 sensor takes reading each two seconds. so the delay between each loop must be 2 seconds or more else it will give the same reading
  }


  //delay(500);

  avgHumidity = avgHumidity / TAKE_AVERAGE_OF_X_READINGS;
  return avgHumidity;
}

//Average temperature of x reading
long takeTemperatureReading() {
  //return DHT.temperature;
  int sumTemperature = 0;
  int avgTemperature = 0;

  for (int i = 0; i < TAKE_AVERAGE_OF_X_READINGS; i++)
  {
    avgTemperature += DHT.temperature;
    delay(2200);  //DHT22 sensor takes reading each two seconds. so the delay between each loop must be 2 seconds or more else it will give the same reading
  }


  //delay(500);

  avgTemperature = avgTemperature / TAKE_AVERAGE_OF_X_READINGS;
  return avgTemperature;
}





//Printing on Nokia Screen
void draw(long moisture_one, long moisture_two, long sunlight, long temperature, long humidity, String rTime) {
  display.setFont(u8g_font_profont11);  // select font

  //Display Temprature Sensor reading
  display.setPrintPos(0, 10);  // set position
  display.print("T:" + String(temperature) + "C");

  display.setPrintPos(45, 10);  // set position
  display.print("H:" + String(humidity) + "%");

  display.setPrintPos(0, 22);  // set position
  display.print("M1:" + String(moisture_one) + "%");

  display.setPrintPos(45, 22);  // set position
  display.print("M2:" + String(moisture_two) + "%");

  display.setPrintPos(0, 32);  // set position
  display.print("L:" + String(sunlight) + "%");

  display.setPrintPos(35, 32);  // set position
  display.print("LR:" + String(rTime));

  int v1State = digitalRead(RELAY_VALVE_ONE);
  int v2State = digitalRead(RELAY_VALVE_TWO);
  String v1Status; String v2Status;

  if (v1State == 0) {
    v1Status = "CLS";
  } else {
    v1Status = "OPN";
  }

  if (v2State == 0) {
    v2Status = "CLS";
  } else {
    v2Status = "OPN";
  }

  display.setPrintPos(0, 45);  // set position
  display.print("V1:" + String(v1Status));

  display.setPrintPos(45, 45);  // set position
  display.print("V2:" + String(v2Status));

}



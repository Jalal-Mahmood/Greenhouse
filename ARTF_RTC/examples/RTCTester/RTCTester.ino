/*
  RTC Tester

  Test Set/Read RTC date/time using simple Serial interface.

  Notes:
  - Date/time currently displayed as unix timestamp
  - Date/time is UTC timezone

  Created 14 6 2014
  Modified 22 6 2014
*/
#include <ARTF_RTC.h>

// UA Sensors RTC - Dependencies
#include <SPI.h>
#include <Time.h>

// Pin connected to RTC CS
const int CS_PIN = 8;

// Instantiate RTC object with CS Pin. Need to call begin before using.
ARTF_RTC rtc(CS_PIN);


// Menu options recognized from Serial input
const char SHOW_MENU = 'm';
const char READ_DATETIME = 'r';
const char SET_DATETIME = 's';
const char SHOW_FORMAT = 'f';


void setup()
{
  Serial.begin(9600);

  // Show menu
  doShowMenu();

  // Call begin to initialize RTC
  rtc.begin();
}

void loop()
{
  if (Serial.available())
  {
    switch (Serial.read())
    {
      case SHOW_MENU:
        doShowMenu();
        break;
      case SHOW_FORMAT:
        doShowDateTimeFormat();
        break;
      case READ_DATETIME:
        doReadDateTime();
        break;
      case SET_DATETIME:
        doSetDateTime();
        break;
    }
  }
}

void doShowMenu()
{
  Serial.println("RTC Tester");
  Serial.println("----------");
  Serial.println("m: Show menu");
  Serial.println("r: Reads current date/time from RTC");
  Serial.println("f: Displays expected date/time format");
  Serial.println("s: Enters mode for setting date/time");
  Serial.println();
}

// Shows expected format for setting date/time.
void doShowDateTimeFormat()
{
  Serial.println("Date Time Format");
  Serial.println("----------------");
  Serial.println("General format: year-month-day hour:minute:second");
  Serial.println("Example format: 2014-01-12 01:05:23");
  Serial.println();
}

// Displays date/time in UNIX timestamp format.
//
// Notes:
// - Function to display readable date/time not implemented yet.
//
void doReadDateTime()
{
  Serial.println("RTC time is: " + rtc.readDateTimeAsText());
  Serial.println();
}

// Asks user to input date/time via Serial.
//
// Notes:
// - Date/time format must be exact
// - Assumes UTC time
// - Use 'r' command to show updated date/time
//
void doSetDateTime()
{
  char buffer[25];
  int inputLength = 0;

  Serial.println("Enter date time (ex: 2014-01-31 23:05:23):");

  // Wait for user to provie date/time via Serial
  while (!Serial.available())
  {
    delay(100);
  }

  // Read input until newline character is seen. May need
  // to adjust serial console accordingly
  inputLength = Serial.readBytesUntil('\n', buffer, 25);
  buffer[inputLength] = '\0';

  // Expects exactly 19 characters, otherwise
  // assumes that date/time was entered incorrectly.
  if (inputLength != 19)
  {
    Serial.println("Error: Incorrect date format!\n");
    return;
  }

  // Convert buffer to String to simplify extraction
  // of date/time values.
  String input = String(buffer);

  Serial.println("Changing time to: " + input + "...");

  int year = input.substring(0, 4).toInt();
  int month = input.substring(5, 7).toInt();
  int day = input.substring(8, 10).toInt();
  int hour = input.substring(11, 13).toInt();
  int minute = input.substring(14, 16).toInt();
  int second = input.substring(17, 19).toInt();

  // Set date/time from user input. Time library expects year as number
  // years since 1970.
  rtc.setDateTime(day, month, year - 1970, hour, minute, second);

  Serial.println("RTC time set to: " + rtc.readDateTimeAsText());
  Serial.println();

}

/*
  SDCard Tester

  Test SD Card read/write functionality with simple Serial interface.

  Notes:
  - Behind the scenes uses SD Fat library instead of the default SD library
  - File clear not implemented yet
  - Write file appends to existing content

  Created 14 6 2014
  Modified 22 6 2014
*/
#include <ARTF_SDCard.h>

// UA Sensors SDCard - Dependencies
#include <SdFat.h>
#include <String.h>

// Pin connected to SD Card CS
const int CS_PIN = 10;

// Maximum number of characters allowed for message input
// Includes one extra slot for null byte terminator.
const int MESSAGE_SIZE = 101;

// Set the name of the file to be read.
#define FILENAME "testfile.txt"

// Instantiate SDCard object. Need to call begin before using.
ARTF_SDCard sd(CS_PIN);


// Menu options recognized from Serial input
const char SHOW_MENU = 'm';
const char READ_FILE = 'r';
const char WRITE_FILE = 'w';


void setup()
{
  Serial.begin(9600);

  doShowMenu();

  // Call begin to initialize SD Card
  sd.begin();
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
      case READ_FILE:
        doReadFile();
        break;
      case WRITE_FILE:
        doWriteFile();
        break;
    }
  }
}

void doShowMenu()
{
  Serial.println("SD Card Tester commands");
  Serial.println("-----------------------");
  Serial.println("m: Show menu");
  Serial.println("r: Reads contents of file");
  Serial.println("w: Write contents to file");
  Serial.println();
}

// Reads contents from specified file and returns
// the contents as String that will be printed to
// Serial console.
void doReadFile()
{
  String contents = sd.readFile(FILENAME);

  if (contents.length() > 0)
  {
    Serial.println("Contents of " + String(FILENAME) + ":");
    Serial.println(contents);
    Serial.println();
  }
  else
  {
    Serial.println("Notice: No contents found on file.\n");
  }
}


void doWriteFile()
{
  char buffer[MESSAGE_SIZE];
  int inputLength = 0;

  Serial.print("Write a message up to ");
  Serial.print(MESSAGE_SIZE - 1);
  Serial.println(" characters:");

  // Wait for user to enter a message
  while (!Serial.available())
  {
    delay(100);
  }

  // Read input until newline character is seen. May need
  // to adjust serial console accordingly
  inputLength = Serial.readBytesUntil('\n', buffer, MESSAGE_SIZE);
  buffer[inputLength] = '\0';

  if (inputLength == 0)
  {
    Serial.println("Error: No valid input was found!\n");
    return;
  }

  // Convert char string to string. SDCard writeFile method expects String.
  String message = String(buffer);

  Serial.println("Writing to " + String(FILENAME) + ":");
  Serial.println(message);
  Serial.println();

  bool isSuccess = sd.writeFile(FILENAME, message);

  if (isSuccess)
  {
    Serial.println("Message written to file successfully!\n");
  }
  else
  {
    Serial.println("Error: Could not write message to file!\n");
  }
}


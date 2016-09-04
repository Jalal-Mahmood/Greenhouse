// Implementation based on example code from Sparkfun: https://github.com/sparkfun/DeadOn_RTC

#include <ARTF_RTC.h>
#include <Arduino.h>
#include <SPI.h>
#include <String.h>

void ARTF_RTC::begin()
{
    pinMode(_csPin, OUTPUT);

    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE3);

    digitalWrite(_csPin, LOW);
    SPI.transfer(0x8E);
    SPI.transfer(0x60);
    digitalWrite(_csPin, HIGH);
    delay(10);
}

void ARTF_RTC::setDateTime(int day, int month, int year, int hours, int minutes, int seconds)
{
    int timedate[7] = { seconds, minutes, hours, 0, day, month, year };
    for (int i = 0; i <= 6; ++i)
    {
        if (i == 3)
        {
            i++;
        }

        int b = timedate[i] / 10;
        int a = timedate[i] - b * 10;

        if (i == 2)
        {
            if (b == 2)
            {
                b = B00000010;
            }
            else if (b == 1)
            {
                b = B00000001;
            }
        }

        timedate[i] = a + (b << 4);
        digitalWrite(_csPin, LOW);
        SPI.transfer(i+0x80);
        SPI.transfer(timedate[i]);
        digitalWrite(_csPin, HIGH);
    }
}

String ARTF_RTC::readDateTimeAsText()
{
    time_t timestamp = readDateTime();
    return String(year(timestamp)) + "-" + String(month(timestamp)) + "-" + String(day(timestamp)) + " " +
        String(hour(timestamp)) + ":" + String(minute(timestamp)) + ":" + String(second(timestamp));
}

String ARTF_RTC::readDateAsText()
{
    time_t timestamp = readDateTime();
    return String(year(timestamp)) + "-" + String(month(timestamp)) + "-" + String(day(timestamp));
}

String ARTF_RTC::readTimeAsText()
{
    time_t timestamp = readDateTime();
    return String(hour(timestamp)) + ":" + String(minute(timestamp));
}

time_t ARTF_RTC::readDateTime()
{
    int timedate[7];
    for (int i = 0; i <= 6; ++i)
    {
        if (i == 3)
        {
            i++;
        }

        digitalWrite(_csPin, LOW);
        SPI.transfer(i + 0x00);
        unsigned int n = SPI.transfer(0x00);
        digitalWrite(_csPin, HIGH);
        int a = n & B00001111;

        if (i == 2)
        {
            int b = (n & B00110000) >> 4;
            if (b == B00000010)
            {
                b = 20;
            }
            else if (b == B00000001)
            {
                b = 10;
            }
            timedate[i] = a + b;
        }
        else if (i == 4)
        {
            int  b = (n & B00110000) >> 4;
            timedate[i] = a + b * 10;
        }
        else if (i == 5)
        {
            int b = (n & B00010000) >> 4;
            timedate[i] = a + b * 10;
        }
        else if (i == 6)
        {
            int b = (n & B11110000) >> 4;
            timedate[i] = a + b * 10;
        }
        else
        {
            int b = (n & B01110000) >> 4;
            timedate[i] = a + b * 10;
        }
    }

    TimeElements tm;
    tm.Year = timedate[6];
    tm.Month = timedate[5];
    tm.Day = timedate[4];
    tm.Hour = timedate[2];
    tm.Minute = timedate[1];
    tm.Second = timedate[0];

    return makeTime(tm);
}
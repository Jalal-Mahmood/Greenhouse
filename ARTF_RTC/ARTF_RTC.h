#ifndef _ARTF_RTC_H_
#define _ARTF_RTC_H_

#include <Arduino.h>
#include <String.h>
#include <Time.h>

class ARTF_RTC {
public:
    ARTF_RTC(int csPin) : _csPin(csPin) {};
    void begin();
    void setDateTime(int day, int month, int year, int hours, int minutes, int seconds);
    String readDateTimeAsText();
	String readDateAsText();
	String readTimeAsText();
    time_t readDateTime();
private:
    int _csPin;
};

#endif
#include "print.h"
#include <arduino.h>

// *.cpp file

void print(int number) {
    Serial.print(char(number));
};

void println(int number) {
    Serial.println(char(number));
};
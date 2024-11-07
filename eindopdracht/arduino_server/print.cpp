#include "print.h"
#include <arduino.h>

// *.cpp file

void print(int number) {
    Serial.print(char(number));
};
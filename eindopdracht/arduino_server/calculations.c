#include "calculations.h"
#include <math.h>
#include <stdio.h>

calculations* calculationsInit() {
  calculations* calc = malloc(sizeof(calculations));
  calc->sum = 0;
  calc->total = 0;
  calc->sumkwad = 0;
  return calc;
};

double calcAddValue(calculations* calc, double value) {
  calc->sum += value;
  calc->total++;
  calc->sumkwad += pow(value, 2);

  return value;
}

double calcReset(calculations* calc) {
  calc->sum = 0;
  calc->total = 0;
  calc->sumkwad = 0;
  return 0;
}

/*long term average of buffer values even when the values
 * have been read/overwritten*/
double runningAvg(calculations* calc) {
  if (calc == NULL || calc->total == 0) {
    return -1;
  }
  return calc->sum / calc->total;
};

/*Standard deviation of the buffer even when values have
 * been read/overwritten*/
double deviation(calculations* calc) {
  if (calc == NULL || calc->total == 0) {
    return -1;
  }
  return sqrt((calc->sumkwad -
               calc->sum * calc->sum / calc->total) /
              calc->total);
};
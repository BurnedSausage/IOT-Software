#include <stdlib.h>

typedef struct {
  double sum;
  double total;
  double sumkwad;
} calculations;

calculations* calculationsInit();

double calcAddValue(calculations* calc, double value);

double calcReset(calculations* calc);

/*long term average of buffer values even when the values
 * have been read/overwritten*/
double runningAvg(calculations* calc);

/*Standard deviation of the buffer even when values have
 * been read/overwritten*/
double deviation(calculations* calc);
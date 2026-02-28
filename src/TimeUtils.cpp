#include <stdint.h>     // ← Add this line to define uint8_t
#include "TimeUtils.h"

void epochToDateTime(unsigned long epoch, int& year, int& month, int& day, int& hour, int& minute, int& second) {
  second = epoch % 60;
  epoch /= 60;
  minute = epoch % 60;
  epoch /= 60;
  hour   = epoch % 24;
  epoch /= 24;

  year = 1970;
  while (true) {
    unsigned long daysInYear = 365;
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
      daysInYear = 366;
    }
    if (epoch < daysInYear) break;
    epoch -= daysInYear;
    year++;
  }

  static const uint8_t daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  month = 0;
  while (true) {
    uint8_t dim = daysInMonth[month];
    if (month == 1 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
      dim = 29;
    }
    if (epoch < dim) break;
    epoch -= dim;
    month++;
  }
  month++;
  day = epoch + 1;
}
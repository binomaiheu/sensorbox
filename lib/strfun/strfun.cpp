#include "strfun.h"

String Float2String(const double value) {
  return Float2String(value, 2);
}

String Float2String(const double value, uint8_t digits) {
  // Convert a float to String with two decimals.
  char temp[15];

  dtostrf(value, 13, digits, temp);
  String s = temp;
  s.trim();
  return s;
}

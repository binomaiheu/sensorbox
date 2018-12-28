#pragma once

#include <Arduino.h>

String Float2String(const double value);
String Float2String(const double value, uint8_t digits);

char *ltrim(char *str, const char *seps);
char *rtrim(char *str, const char *seps);
char *trim(char *str, const char *seps);

#pragma once

#include <cactus_io_SHT15.h>
#include <sds011.h>

// Define the Sensirion SHT15 sensor and it's pins...
#define SHT15_dataPin   2
#define SHT15_clockPin  3

SHT15 sht(SHT15_dataPin, SHT15_clockPin);


// Define the SDS011 sensor and it's pins...
#define SDS011_RX 5
#define SDS011_TX 6

sds011 sds( SDS011_RX, SDS011_TX );
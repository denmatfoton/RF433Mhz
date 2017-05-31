#ifndef _SIGNALANALYSER_H
#define _SIGNALANALYSER_H

#include <stdint.h>

#define SHORT_PULSE 0
#define LONG_PULSE  1
#define UNDEF_PULSE 2

bool analyseData(uint32_t* data, uint32_t dataLength, uint32_t dispersion,
                    uint32_t minPulseLength, uint32_t& shortPulse, uint32_t& longPulse);

#endif // _SIGNALANALYSER_H

#ifndef _433RFSNIFER_H
#define _433RFSNIFER_H

#include <stdio.h>
#include <stdint.h>

#define DATA_PIN                 2
#define TRANSMITTER_PIN          0
#define MAX_CHANGE_TO_KEEP       1024
#define MAX_SIGNAL_LENGTH        256
#define RECEIVED_CODES_MAX_COUNT 32
#define SKIP_PULSES 6

#define OPERATION_PLAY_RFCODE    0
#define OPERATION_WRITE_RFCODE   1
#define OPERATION_CLOSE          2

#define DEFAULT_RFCODES_FILE_NAME "rfcodes.bin"

#define incr(x)      x = (x + 1) % MAX_CHANGE_TO_KEEP
#define decr(x, y)  ((x + MAX_CHANGE_TO_KEEP - y) % MAX_CHANGE_TO_KEEP)


struct AnalyseConfig
{
    uint32_t dispersion;
    uint32_t period;
    uint32_t valuesCount;
    uint32_t minPulseLength;
    uint32_t delayAfterDecodedSignal;
};

#endif // _433RFSNIFER_H
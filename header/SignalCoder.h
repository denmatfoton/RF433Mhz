#ifndef _SIGNALCODER_H
#define _SIGNALCODER_H

#include <stdint.h>

#define SIGNAL_DECODING_DISPERSION_IN_PERCENT 30
#define SYNC_SIGNAL_MAX_LENGTH                4
#define RFCODE_REPITE_COUNT                   10

class RFCode
{
public:
    void print();
    uint32_t shortPulse;
    uint32_t longPulse;
    uint32_t syncSig[SYNC_SIGNAL_MAX_LENGTH];
    uint64_t data;
    uint32_t dataLength;
};

bool decodeSignal(uint32_t* data, uint32_t dataLength,
                    uint32_t shortPulse, uint32_t longPulse, RFCode* rfCode);
                    
void sendRFCode(RFCode* rfCode, const uint32_t transmitterPin);

#endif // _SIGNALCODER_H

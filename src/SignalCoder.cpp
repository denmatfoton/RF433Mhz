#include <iostream>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include "SignalCoder.h"
#include "SignalAnalyser.h"

using namespace std;

void RFCode::print(void)
{
    printf("RFCode: \n");
    printf("short, long pulse: %u %u\n", this->shortPulse, this->longPulse);
    printf("sync signal:");
    for (uint32_t i = 0; i < SYNC_SIGNAL_MAX_LENGTH && this->syncSig[i] > 0; ++i)
    {
        printf(" %u", this->syncSig[i]);
    }
    printf("\ndata: %llu\n", this->data);
    printf("data length: %u\n", this->dataLength);
}

static void optimiseRFCode(RFCode* rfCode)
{
    uint32_t lVs = (uint32_t)ceil((double)rfCode->longPulse / rfCode->shortPulse - 0.5d);
    uint32_t sum = rfCode->longPulse + rfCode->shortPulse;
    
    rfCode->shortPulse = sum / (lVs + 1);
    rfCode->longPulse = rfCode->shortPulse * lVs;
    
    for (uint32_t j = 0; j < SYNC_SIGNAL_MAX_LENGTH && rfCode->syncSig[j] > 0;  j++)
    {
        rfCode->syncSig[j] = (uint32_t)ceil((double)rfCode->syncSig[j] / rfCode->shortPulse - 0.5d) * rfCode->shortPulse;
    }
}

static bool decodeSyncro(uint32_t* data, uint32_t dataLength, uint32_t startData, uint32_t endData, RFCode* rfCode)
{    
    uint32_t syncLength = dataLength - endData + startData;
    uint32_t i = 0, j;
    
    if (syncLength > SYNC_SIGNAL_MAX_LENGTH)
    {
        printf("Untypical pulse, synchro > SYNC_SIGNAL_MAX_LENGTH\n");
        printf("length = %u, start = %u, end = %x\n", dataLength, startData, endData);
        return false;
    }
    
    for (; i + endData < dataLength; ++i)
    {
        rfCode->syncSig[i] = data[i + endData];
    }
        
    for (j = i; i < syncLength; ++i)
    {
        rfCode->syncSig[i] = data[i - j];
    }
    
    return true;
}

static bool decodeData(uint32_t* data, uint32_t dataLength, uint32_t& startData, uint32_t& endData, RFCode* rfCode)
{
    uint32_t shortPulse = rfCode->shortPulse;
    uint32_t longPulse = rfCode->longPulse;
    uint32_t dispersion = (longPulse - shortPulse) * SIGNAL_DECODING_DISPERSION_IN_PERCENT / 100;
    uint32_t prevPulse, curPulse;
    uint32_t shortSum = 0, longSum = 0;
    uint64_t code = 0;
    bool firstRun = true;
    
    for (uint32_t i = startData; i < endData; ++i)
    {
        if (abs((int)data[i] - (int)shortPulse) < dispersion) // like short
        {
            shortSum += data[i];
            curPulse = SHORT_PULSE;
        }
        else // like long
        {
            longSum += data[i];
            curPulse = LONG_PULSE;
        }
        
        if ((i - startData) % 2 == 1)
        {
            if (prevPulse != curPulse)
            {
                code <<= 1;
                code |= prevPulse;
            }
            else
            {
                if (firstRun)
                { // try to shift
                    firstRun = false;
                    i = startData++;
                    continue;
                }
                printf("Untypical pulse, same pulses in same bit %u %u\n", startData, i);
                return false;
            }
        }
        else
        {
            prevPulse = curPulse;
        }
    }
    
    if ((endData - startData) % 2 == 1)
    {
        --endData;
    }
    
    rfCode->data = code;
    rfCode->dataLength = (endData - startData) / 2;
#if 1
    rfCode->shortPulse = shortSum / rfCode->dataLength;
    rfCode->longPulse = longSum / rfCode->dataLength;
#endif
    
    return true;
}

bool decodeSignal(uint32_t* data, uint32_t dataLength,
                    uint32_t shortPulse, uint32_t longPulse, RFCode* rfCode)
{
    uint32_t i = 0;
    uint32_t dispersion = (longPulse - shortPulse) * SIGNAL_DECODING_DISPERSION_IN_PERCENT / 100;
    uint32_t startData = 0, endData;
    memset((void*)rfCode, 0, sizeof(RFCode));
    rfCode->shortPulse = shortPulse;
    rfCode->longPulse = longPulse;
    
    for (; i < dataLength; ++i)
    {
        if (abs((int)data[i] - (int)shortPulse) < dispersion ||
                abs((int)data[i] - (int)longPulse) < dispersion)
        {
            if (startData == 0)
            {
                if (i == 0)
                {
                    printf("Untypical signal, no synchro\n");
                    return false;
                }
                startData = i;
            }
        }
        else
        {
            if (startData > 0)
            {
                break;
            }
        }
    }
    
    endData = i;
    
    if (!decodeData(data, dataLength, startData, endData, rfCode))
    {
        return false;
    }
    if (!decodeSyncro(data, dataLength, startData, endData, rfCode))
    {
        return false;
    }
    
    optimiseRFCode(rfCode);
    
    return true;
}


static inline void transmit(uint32_t highPulse, uint32_t lowPulse, uint32_t transmitterPin)
{
    //printf(" %u %u", highPulse, lowPulse);
#if 0
    static int64_t time = micros() + 100;
    int delay = time - micros();
    
    //printf("delay: %d ", delay);
    delayMicroseconds(delay);
    digitalWrite(transmitterPin, HIGH);
    delayMicroseconds(highPulse - 20);
    time = micros() + lowPulse;
    digitalWrite(transmitterPin, LOW);
    //printf("time: %llu ", time);
#else
    digitalWrite(transmitterPin, HIGH);
    delayMicroseconds(highPulse);
    digitalWrite(transmitterPin, LOW);
    delayMicroseconds(lowPulse);
#endif
}

void sendRFCode(RFCode* rfCode, const uint32_t transmitterPin)
{
    const uint64_t code = rfCode->data;
    const uint32_t shortPulse = rfCode->shortPulse;
    const uint32_t longPulse = rfCode->longPulse;
    
    for (uint32_t i = 0; i < RFCODE_REPITE_COUNT; ++i)
    {
        uint32_t j = 0;
        
        for (;j < SYNC_SIGNAL_MAX_LENGTH && rfCode->syncSig[j] > 0;  j += 2)
        {
            transmit(rfCode->syncSig[j], rfCode->syncSig[j + 1], transmitterPin);
        }
        
        for (j = rfCode->dataLength; j > 0;)
        {
            if ((code >> --j) & 0x1)
            { // bit == 1
                transmit(longPulse, shortPulse, transmitterPin);
            }
            else
            { // bit == 0
                transmit(shortPulse, longPulse, transmitterPin);
            }
        }
    }
}

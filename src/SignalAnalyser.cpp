#include <algorithm>
#include <string.h>
#include "SignalAnalyser.h"

using namespace std;

#define MIN_CLASTER_SIZE   (dataLength / 4)


static uint32_t clasterSearch(uint32_t* data, uint32_t dataLength, uint32_t dispersion,
                                uint32_t startPos, uint32_t& clasterPos, uint32_t& clasterMedian)
{
    uint32_t clasterSize;
    uint32_t clasterSum;
    uint32_t i = startPos;
    
    while (i < dataLength)
    {
        clasterSize = 1;
        clasterSum = data[i++];
        
        for (; i < dataLength; ++i)
        {
            if (data[i] - clasterSum / clasterSize < dispersion)
            {
                clasterSum += data[i];
                ++clasterSize;
            }
            else
            {
                break;
            }
        }
        
        if (clasterSize > MIN_CLASTER_SIZE)
        {
            clasterMedian = clasterSum / clasterSize;
            clasterPos = i - clasterSize;
            return (int)clasterSize;
        }
    }
    
    return 0;
}

#define checkPrevious(curPulse)   if (prev == curPulse)         \
                                {                           \
                                    if (sequenceSum == 2)   \
                                    {                       \
                                        return false;       \
                                    }                       \
                                    sequenceSum++;          \
                                }                           \
                                else                        \
                                {                           \
                                    prev = curPulse;          \
                                    sequenceSum = 1;        \
                                }

static bool sanityCheck(uint32_t* data, uint32_t dataLength, uint32_t dispersion,
                            uint32_t shortPulse, uint32_t longPulse)
{
    if ((float)longPulse / shortPulse < 1.9f)
    {
        return false;
    }
    
    int sum = 0;
    int irregPos = -1;
    uint32_t prev = UNDEF_PULSE;
    uint32_t sequenceSum = 0;
    
    for (uint32_t i = 0; i < dataLength; ++i)
    {
        if (abs((int)(data[i] - shortPulse)) < dispersion) // like short
        {
            checkPrevious(SHORT_PULSE)
            --sum;
        }
        else if (abs((int)(data[i] - longPulse)) < dispersion) // like long
        {
            checkPrevious(LONG_PULSE)
            ++sum;
        }
        else
        {
            if (irregPos < 0 || irregPos == (int)i - 1) // skip probable sync signal
            {
                sum = 0;
                prev = UNDEF_PULSE;
            }
            else
            {
                return false;
            }
        }
        
        if (abs((int)sum) > 2)
        {
            return false;
        }
    }
    
    return true;
}


bool analyseData(uint32_t* data, uint32_t dataLength, uint32_t dispersion,
                    uint32_t minPulseLength, uint32_t& shortPulse, uint32_t& longPulse)
{
    uint32_t* sorted = data + dataLength;
    memcpy(sorted, data, sizeof(uint32_t) * dataLength);
    sort(sorted, sorted + dataLength);

    uint32_t clasterSize;
    uint32_t clasterPos;
    uint32_t clasterMedian;
    uint32_t startPos = 0;
    shortPulse = longPulse = 0;
    
    // short search
    clasterSize = clasterSearch(sorted, dataLength, dispersion, startPos, clasterPos, clasterMedian);
    startPos = clasterPos + clasterSize;
    if ((clasterSize > 0) && (startPos < dataLength - MIN_CLASTER_SIZE))
    {
        shortPulse = clasterMedian;
    }
    else
    {
        return false;
    }
    
    if (shortPulse < minPulseLength)
    {
        return false;
    }
    
    // long search
    clasterSize = clasterSearch(sorted, dataLength, dispersion, startPos, clasterPos, clasterMedian);
    if (clasterSize > 0)
    {
        longPulse = clasterMedian;
    }
    else
    {
        return false;
    }
    
    return sanityCheck(data, dataLength, dispersion, shortPulse, longPulse);
}

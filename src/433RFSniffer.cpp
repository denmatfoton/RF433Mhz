#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include <wiringPi.h>
#include "433RFSniffer.h"
#include "SignalAnalyser.h"
#include "SignalCoder.h"
#include "StoreRFCode.h"

using namespace std;


static uint32_t durations[MAX_CHANGE_TO_KEEP];
static uint32_t dur_pos = 0;
static RFCode receivedCodes[RECEIVED_CODES_MAX_COUNT];
static uint32_t curReceivedCodesCount = 0;

static sem_t analyseSem;
static bool endAnalyseThread = false;
static bool skipSignal = false;
static bool signalDetected = false;
static uint32_t synchroThreshold;
static uint32_t changeCount;
static AnalyseConfig analyseConfig = {  50,     // pulse dispersion
                                        100,    // check period
                                        40,     // analysing data length
                                        150,    // minimal pulse duration
                                        500000  // delay 500 ms after decoded signal
                                        };


static void printArray(uint32_t* array, uint32_t length)
{
    for (uint32_t i = 0; i < length; ++i)
    {
        printf("%d ", (int)array[i]);
    }
    
    printf("\n");
}

void* analyseRoutine(void* args)
{
    static uint32_t analysePos;
    static uint32_t data[MAX_SIGNAL_LENGTH];
    static uint32_t shortPulse, longPulse;
    static uint64_t lastDecodedSignalTime;
    static uint32_t j;
    bool firstRun = true;
    static RFCode rfCode;
    
    while (true)
    {
        sem_wait(&analyseSem);
        
        if (endAnalyseThread)
        {
            break;
        }
        
        if (!signalDetected)
        {
            if (micros() - lastDecodedSignalTime < analyseConfig.delayAfterDecodedSignal)
            {
                continue;
            }
            
            analysePos = decr(dur_pos, analyseConfig.valuesCount);
            
            for (uint32_t i = 0; i < analyseConfig.valuesCount; ++i)
            {
                data[i] = durations[analysePos];
                incr(analysePos);
            }
            
            if (analyseData(data, analyseConfig.valuesCount, analyseConfig.dispersion,
                                analyseConfig.minPulseLength, shortPulse, longPulse))
            {
                synchroThreshold = longPulse * 2;
                signalDetected = true;
                changeCount = SKIP_PULSES;
            }
        }
        else
        {
            analysePos = decr(dur_pos, 2);
            
            if (firstRun)
            {
                firstRun = false;
                j = analysePos;
                continue;
            }
            firstRun = true;
            signalDetected = false;
            
            while (durations[j] > synchroThreshold)
            {
                j = decr(j, 1);
            }
            
            uint32_t possibleSignalLength = decr(analysePos, j);
            
            if (possibleSignalLength >= MAX_SIGNAL_LENGTH)
            {
                printf("abort 1\n");
                continue;
            }
            if (possibleSignalLength % 2 != 0)
            {
                printf("abort 2\n");
                continue;
            }
            
            uint32_t i = 0;
            
            for (; i < possibleSignalLength; ++i)
            {
                incr(j);
                data[i] = durations[j];
            }
            
            printArray(data, i);
            if (decodeSignal(data, i, shortPulse, longPulse, &rfCode))
            {
                rfCode.print();
                if (curReceivedCodesCount < RECEIVED_CODES_MAX_COUNT)
                {
                    receivedCodes[curReceivedCodesCount] = rfCode;
                    printf("RFCode index: %u\n", curReceivedCodesCount);
                    ++curReceivedCodesCount;
                }
            }
            
            lastDecodedSignalTime = micros();
        }
    }

    return NULL;
}


void handleInterrupt(void)
{
    static uint64_t lastTime;

    if (skipSignal)
    {
        return;
    }
    
    uint64_t time = micros();
    durations[dur_pos] = (uint32_t)(time - lastTime);
    incr(dur_pos);

    if (!signalDetected)
    {
        if (changeCount++ > analyseConfig.period)
        {
            sem_post(&analyseSem);
            changeCount = 0;
        }
    }
    else
    {
        if (++changeCount > SKIP_PULSES &&
                durations[decr(dur_pos, 1)] > synchroThreshold)
        {   
            sem_post(&analyseSem);
            changeCount = 0;
        }
    }

    lastTime = time;
}


int main(void) {
    pthread_t analyseThread;
    
    if(wiringPiSetup() == -1)
    {
        return 0;
    }

    sem_init(&analyseSem, 0, 0);
    pthread_create(&analyseThread, NULL, &analyseRoutine, NULL);

    //attach interrupt to changes on the pin
    wiringPiISR(DATA_PIN, INT_EDGE_BOTH, &handleInterrupt);

    while (true)
    {
        uint32_t operation, param;
        scanf("%u", &operation);
        
        switch (operation)
        {
        case OPERATION_PLAY_RFCODE:
            printf("RFCode index: ");
            scanf("%u", &param);
            if (param < RECEIVED_CODES_MAX_COUNT)
            {
                skipSignal = true;
                sendRFCode(&receivedCodes[param], TRANSMITTER_PIN);
                skipSignal = false;
                printf("code sent\n");
            }
            break;
        case OPERATION_WRITE_RFCODE:
            {
                char rfCodeName[MAX_RFCODE_NAME_LENGTH];
                
                printf("RFCode name: ");
                scanf("%s", rfCodeName);
                printf("RFCode index: ");
                scanf("%u", &param);
                if (param >= RECEIVED_CODES_MAX_COUNT)
                {
                    continue;
                }
                if (addRFCodeToFile(DEFAULT_RFCODES_FILE_NAME, &receivedCodes[param], rfCodeName))
                {
                    printf("code written\n");
                }
                else
                {
                    printf("error occured\n");
                }
            }
            break;
        default:
            break;
        }
        
        if (operation == OPERATION_CLOSE)
        {
            break;
        }
    }
    
    endAnalyseThread = true;
    sem_post(&analyseSem);
    pthread_join(analyseThread, NULL);
    
    return 0;
}

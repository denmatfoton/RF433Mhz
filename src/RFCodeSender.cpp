#include <stdlib.h>
#include <wiringPi.h>
#include "RFCodeSender.h"
#include "StoreRFCode.h"
#include "SignalCoder.h"

int main(int argc, char *argv[])
{
    char* codeName;
    char defaultFileName[] = DEFAULT_RFCODES_FILE_NAME;
    char* fileName = defaultFileName;
    RFCode rfCode;

    if(wiringPiSetup() == -1)
    {
        return 0;
    }

    if (argc < 2)
    {
        printf("Need code name parameter\n");
        return 0;
    }
    codeName = argv[1];
    if (argc > 2)
    {
        fileName = argv[2];
    }
    
    if (!readRFCodeFromFile(fileName, &rfCode, codeName))
    {
        return 0;
    }
    
    pinMode(TRANSMITTER_PIN, OUTPUT);
    sendRFCode(&rfCode, TRANSMITTER_PIN);
    
    return 0;
}

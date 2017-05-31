#ifndef _STORERFCODE_H
#define _STORERFCODE_H

#include "SignalCoder.h"

#define MAX_RFCODE_NAME_LENGTH 32
#define FILE_ELEMENT_SIZE      (MAX_RFCODE_NAME_LENGTH + sizeof(RFCode))

bool addRFCodeToFile(const char* fileName, RFCode* rfCode, char* rfCodeName);
bool readRFCodeFromFile(const char* fileName, RFCode* rfCode, const char* rfCodeName);

#endif // _STORERFCODE_H

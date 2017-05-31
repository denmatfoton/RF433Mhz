#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "StoreRFCode.h"

bool addRFCodeToFile(const char* fileName, RFCode* rfCode, char* rfCodeName)
{
    FILE * pFile;
    char buffer[FILE_ELEMENT_SIZE];
    uint32_t fileSize;
    
    pFile = fopen(fileName, "ab+");
    
    fseek(pFile, 0, SEEK_END);
    fileSize = ftell(pFile);
    
    for (uint32_t i = 0; i < fileSize; i += FILE_ELEMENT_SIZE)
    {
        fseek(pFile, i, SEEK_SET);
        fread(buffer, sizeof(char), MAX_RFCODE_NAME_LENGTH, pFile);
        if (strcmp(rfCodeName, buffer) == 0)
        {
            printf("Name already exists\n");
            return false;
        }
    }
    
    memset(buffer, 0, FILE_ELEMENT_SIZE);
    memcpy(buffer, rfCodeName, strlen(rfCodeName));
    memcpy(buffer + MAX_RFCODE_NAME_LENGTH, rfCode, sizeof(RFCode));
    
    fwrite(buffer, sizeof(char), FILE_ELEMENT_SIZE, pFile);
    fclose(pFile);
    
    return true;
}

bool readRFCodeFromFile(const char* fileName, RFCode* rfCode, const char* rfCodeName)
{
    FILE * pFile;
    char buffer[FILE_ELEMENT_SIZE];
    uint32_t fileSize;
    uint32_t i = 0;
    
    pFile = fopen(fileName, "rb");
    if (pFile == NULL)
    {
        printf("File not found\n");
        return false;
    }
    
    fseek(pFile, 0, SEEK_END);
    fileSize = ftell(pFile);
    
    for (; i < fileSize; i += FILE_ELEMENT_SIZE)
    {
        fseek(pFile, i, SEEK_SET);
        fread(buffer, sizeof(char), FILE_ELEMENT_SIZE, pFile);
        if (strcmp(rfCodeName, buffer) == 0)
        {
            break;
        }
    }
    
    fclose(pFile);
    
    if (i >= fileSize)
    {
        printf("Name not found\n");
        return false;
    }
    
    memcpy(rfCode, buffer + MAX_RFCODE_NAME_LENGTH, sizeof(RFCode));
    
    return true;
}

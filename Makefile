INCLUDE = -Iheader/
CC = g++
CFLAGS = -c -Wall
LIBS = -lwiringPi -lpthread
        
SOURCES_FOR_SNIFER = src/433RFSniffer.cpp
SOURCES_FOR_SNIFER += src/SignalAnalyser.cpp
SOURCES_FOR_SNIFER += src/SignalCoder.cpp
SOURCES_FOR_SNIFER += src/StoreRFCode.cpp

SOURCES_FOR_CONTROLLER = src/RFCodeSender.cpp
SOURCES_FOR_CONTROLLER += src/SignalCoder.cpp
SOURCES_FOR_CONTROLLER += src/StoreRFCode.cpp

OBJECTS_FOR_SNIFER = $(SOURCES_FOR_SNIFER:.cpp=.o)
OBJECTS_FOR_CONTROLLER = $(SOURCES_FOR_CONTROLLER:.cpp=.o)

all: 433RFSniffer RFCodeSender
	
433RFSniffer: $(OBJECTS_FOR_SNIFER)
	$(CC) $(LIBS) $(OBJECTS_FOR_SNIFER) -o $@
    
RFCodeSender: $(OBJECTS_FOR_CONTROLLER)
	$(CC) $(LIBS) $(OBJECTS_FOR_CONTROLLER) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@
    
clean:
	rm -f ./src/*.o 433RFSniffer RFCodeSender


// -----------------------------------------------------------------------------------
// non-volatile storage base class

#pragma once

#include "Arduino.h"

class NonVolatileStorage {
  public:
    // prepare EEPROM/FLASH/etc. for operation
    virtual bool init(uint16_t size);

    // call frequently to perform any operations that need to happen in the background
    virtual void poll();

    // returns true if all data in any cache has been written
    virtual bool committed();

    // returns true if all data in nv has passed ongoing validation checks
    bool valid();

    // causes write and update to actually write to EEPROM
    // normally both write and update read first and only write if the
    // value has changed (and possibly later after being cached)
    void writeThrough(bool state);

    // reads data from memory
    uint8_t readFromCache(uint16_t i);

    void writeToCache(uint16_t i, uint8_t j);

    // read unsigned char at position i
    uint8_t read(uint16_t i);

    // read unsigned char at position i
    uint8_t readUC(uint16_t i);
    // read signed char at position i
    int8_t readC(uint16_t i);
    // read unsigned int (16bit) starting at position i
    uint16_t readUI(uint16_t i);
    // read signed int (16bit) starting at position i
    int16_t readI(uint16_t i);
    // read unsigned long (32bit) starting at position i
    uint32_t readUL(uint16_t i);
    // read signed long (32bit) starting at position i
    int32_t readL(uint16_t i);
    // read float (32bit) starting at position i
    float readF(uint16_t i);
    // read double (32 or 64bit) starting at position i
    double readD(uint16_t i);
    // read char array (maxLen up to 64 bytes) starting at position i
    void readStr(uint16_t i, char* j, int16_t maxLen);

    // write value j starting at position i
    inline void write(uint16_t i,  uint8_t j) { update (i,j); }
    inline void write(uint16_t i,     char j) { update (i,j); }
    inline void write(uint16_t i,   int8_t j) { update (i,j); }
    inline void write(uint16_t i, uint16_t j) { update (i,j); }
    inline void write(uint16_t i,  int16_t j) { update (i,j); }
    inline void write(uint16_t i, uint32_t j) { update (i,j); }
    inline void write(uint16_t i,  int32_t j) { update (i,j); }
    inline void write(uint16_t i,    float j) { update (i,j); }
    inline void write(uint16_t i,   double j) { update (i,j); }
    inline void write(uint16_t i,    char* j) { update (i,j); }

    // update value j starting at position i
    inline void update(uint16_t i,  uint8_t j) { writeToCache(i, j); }
    inline void update(uint16_t i,     char j) { update(i,(uint8_t)j); }
    inline void update(uint16_t i,   int8_t j) { update(i,(uint8_t)j); }
    inline void update(uint16_t i, uint16_t j) { updateBytes(i, (uint8_t*)&j, sizeof(uint16_t)); }
    inline void update(uint16_t i,  int16_t j) { updateBytes(i, (uint8_t*)&j, sizeof(int16_t)); }
    inline void update(uint16_t i, uint32_t j) { updateBytes(i, (uint8_t*)&j, sizeof(uint32_t)); }
    inline void update(uint16_t i,  int32_t j) { updateBytes(i, (uint8_t*)&j, sizeof(int32_t)); }
    inline void update(uint16_t i,    float j) { updateBytes(i, (uint8_t*)&j, sizeof(float)); }
    inline void update(uint16_t i,   double j) { updateBytes(i, (uint8_t*)&j, sizeof(double)); }
    inline void update(uint16_t i,    char* j) { updateBytes(i, (uint8_t*)&j, strlen(j) + 1); }

    // read count bytes (up to 64) starting at position i into value j
    // for char arrays a negative count represents the maximum length read (if a terminating null is not found)
    void readBytes(uint16_t i, void *j, int16_t count);
    // update count bytes (up to 64) starting at position i from value j
    void updateBytes(uint16_t i, void *j, int16_t count);
    // write count bytes (up to 64) starting at position i from value j
    inline void writeBytes(uint16_t i, void *j, int16_t count) { updateBytes(i, j, count); }

    // NV size in bytes
    uint16_t size = 0;

  protected:
    // returns false if ready to read or write immediately
    virtual bool busy();

    // read byte at position i from storage
    virtual uint8_t readFromStorage(uint16_t i);

    // write value j to position i in storage 
    virtual void writeToStorage(uint16_t i, uint8_t j);

    bool writeOnUpdate = false;

    uint16_t cacheIndex = -1;
    uint16_t cacheSize = 0;
    uint8_t* cache;
    uint16_t cacheStateSize = 0;
    uint8_t* cacheStateRead;
    uint8_t* cacheStateWrite;
};

#define NVS NonVolatileStorage

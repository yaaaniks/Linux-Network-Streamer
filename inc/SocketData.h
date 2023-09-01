#ifndef SOCKETDATA_H
#define SOCKETDATA_H

#include <stdint.h>

#define CNT_CHL         4
#define NO_DATA_EXIST   (uint32_t)0xFF
#define ERROR_OCCUR     (uint32_t)0xBB
#define DATA_EXIST      (uint32_t)0xAA

typedef struct {
    uint32_t header;
    uint32_t message;
    uint32_t timeStamp;
} CommonPacket;

typedef struct {
    int16_t adcRawValue[CNT_CHL];
    double adcVoltage[CNT_CHL];
    uint16_t azimuthAngle;
    uint16_t elevationAngle;
} StandData;

typedef struct {
    uint8_t command;
    uint32_t commonData;
} InterfaceData;

typedef struct {
    CommonPacket common;
    StandData load;
}__attribute__((packed, aligned(2))) StandDataPacket;

typedef struct {
    CommonPacket common;
    InterfaceData load;
}__attribute__((packed, aligned(2))) InterfaceDataPacket;

extern InterfaceDataPacket interfaceDataPacket;
extern StandDataPacket standDataPacket;

#endif // SOCKETDATA_H
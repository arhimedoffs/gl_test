#pragma once

#define MAX_NAME_LEN 32
#define MAX_VALUE_LEN 32
typedef enum tCommand {
    CMD_EMPTY = 0,
    CMD_ADD,
    CMD_GET,
    CMD_DEL,
    CMD_NOT_EXIST
} tCommand;

typedef struct tCommandParam {
    int slot;
    int port;
    int slotRange; // -1 if range is not used
    int portRange; // -1 if range is not used
    char option[MAX_NAME_LEN+1];
    char value[MAX_VALUE_LEN+1];
} tCommandParam;

int cfgAdd(const char *fName, const tCommandParam *const param);
int cfgGet(const char *fName, const tCommandParam *const param);
int cfgDel(const char *fName, const tCommandParam *const param);
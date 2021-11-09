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

/**
 * Add or modify interface options
 * @return number of successfully modified options, -1 if error occured
 */
int cfgAdd(const char *fName, const tCommandParam *const param);

/**
 * Print option value for selected name
 * @return 1 if success, -1 if error occured
 */
int cfgGet(const char *fName, const tCommandParam *const param);

/**
 * Delete interface options
 * @return 1 if success, -1 if error occured
 */
int cfgDel(const char *fName, const tCommandParam *const param);
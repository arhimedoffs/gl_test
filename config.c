#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#define UNUSED(x) (void)(x)

typedef unsigned int uint;

typedef struct TOption {
    char name[MAX_NAME_LEN+1];
    char value[MAX_VALUE_LEN+1];
} TOption;

typedef struct TInterface {
    int slot;
    int port;
    uint optionsCount;
    uint _optionsReserved;
    TOption *options;
    struct TInterface *pNext;
} TInterface;

TInterface* interfaceCreate(void) {
    TInterface *interface = NULL;
    interface = (TInterface*)calloc(1, sizeof(TInterface));
    if (interface == NULL) {
        return NULL;
    }
    interface->options = (TOption*)calloc(8, sizeof(TOption));
    if (interface->options == NULL) {
        free(interface);
        return NULL;
    }
    interface->_optionsReserved = 8;
    interface->optionsCount = 0;
    return interface;
}

void interfaceDelete(TInterface *interface) {
    if (interface == NULL)
        return;
    if (interface->options != NULL)
        free(interface->options);
    free(interface);
}

void interfaceSetAdd(TInterface *set) {
    UNUSED(set);
}

void interfaceSetGet(TInterface *set) {
    UNUSED(set);
}

/**
 * Read configuration from file
 * @param fname file name
 * @param interfaces pointer to set of readed interfaces
 * @return number of successfuly readed interfaces
 */
int readConfigFromFile(const char *fname, TInterface *interfaces) {
    UNUSED(interfaces);

    FILE *file = fopen(fname, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file for read <%s>\n", fname);
        return -1;
    }
    fclose(file);
    return 0;
}

/**
 * Add or modify interface options
 * @return number of successfully modified options, -1 if error occured
 */
int cfgAdd(const char *fName, const tCommandParam *const param) {
    UNUSED(fName);
    UNUSED(param);
    return 0;
}
int cfgGet(const char *fName, const tCommandParam *const param) {
    UNUSED(fName);
    UNUSED(param);
    return 0;
}
int cfgDel(const char *fName, const tCommandParam *const param) {
    UNUSED(fName);
    UNUSED(param);
    return 0;
}
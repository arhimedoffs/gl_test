#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(x) (void)(x)

#define OPTION_ALLOCATION_STEP 8

typedef struct TOption {
    char name[MAX_NAME_LEN+1];
    char value[MAX_VALUE_LEN+1];
} TOption;

typedef struct TInterface {
    int slot;
    int port;
    int optionsCount;
    int _optionsReserved;
    TOption *options;
    struct TInterface *pNext;
} TInterface;

TInterface* interfaceCreate(void) {
    TInterface *interface = NULL;
    interface = (TInterface*)calloc(1, sizeof(TInterface));
    if (interface == NULL) {
        return NULL;
    }
    interface->options = (TOption*)calloc(OPTION_ALLOCATION_STEP, sizeof(TOption));
    if (interface->options == NULL) {
        free(interface);
        return NULL;
    }
    interface->_optionsReserved = OPTION_ALLOCATION_STEP;
    interface->optionsCount = 0;
    return interface;
}

void interfaceDelete(TInterface *interface) {
    if (interface == NULL)
        return;
    free(interface->options);
    free(interface);
}

/**
 * @retval option index if found, negative othervise
 */
int interfaceOptionFind(const TInterface *interface, const char *optionName) {
    if ((interface == NULL) || (optionName == NULL))
        return -1;
    if (interface->options == NULL)
        return -1;
    
    int i = 0;
    for (i = 0; i < interface->optionsCount; i++) {
        if (strcmp(optionName, interface->options[i].name) == 0)
            break;
    }
    return i;
}

/**
 * @return negative on error, 0 on update in place, positive on add new
 */
// TODO: check actual string values length
int interfaceOptionSet(TInterface *interface, const char *optionName, const char *optionValue) {
    if ((interface == NULL) || (optionName == NULL) || (optionValue == NULL))
        return -1;
    if (interface->options == NULL)
        return -1;
    
    // Search if option already exist
    int i = interfaceOptionFind(interface, optionName);

    int retVal = 0;
    if ((i >= 0) && (i < interface->optionsCount)) {
        // Update existing option
        strcpy(interface->options[i].value, optionValue);
        retVal = 0;
    } else {
        // Add new option
        if (interface->optionsCount >= interface->_optionsReserved) {
            int newReserveCount = interface->_optionsReserved + OPTION_ALLOCATION_STEP;
            TOption *newOptions = (TOption*)realloc(interface->options, newReserveCount*sizeof(TOption));
            if (newOptions == NULL)
                return -1;
            interface->options = newOptions;
            interface->_optionsReserved += OPTION_ALLOCATION_STEP;
        }
        int newIndex = interface->optionsCount++;
        strcpy(interface->options[newIndex].name, optionName);
        strcpy(interface->options[newIndex].value, optionValue);
        retVal = 1;
    }
    return retVal;
}

/**
 * @retval number of deleted elements, negative on error
 */
int interfaceOptionDel(TInterface *interface, const char *optionName) {
    if ((interface == NULL) || (optionName == NULL))
        return -1;
    if (interface->options == NULL)
        return -1;
    
    int optionIndex = interfaceOptionFind(interface, optionName);
    if ((optionIndex < 0) || (optionIndex >= interface->optionsCount)) {
        return 0;
    }
    for (int i  = optionIndex+1; i < interface->optionsCount; i++)
        memcpy(&interface->options[i-1], &interface->options[i], sizeof(TOption));
    memset(&interface->options[--interface->optionsCount], 0, sizeof(TOption));
    return 1;
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
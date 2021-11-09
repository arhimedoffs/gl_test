#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

void interfaceListDelete(TInterface *list) {
    while (list != NULL) {
        TInterface *next = list->pNext;
        interfaceDelete(list);
        list = next;
    }
}

void interfaceListWrite(FILE *file, const TInterface *list) {
    if(file == NULL)
        return;
    while (list != NULL) {
        fprintf(file, "interface %d/%d\n", list->slot, list->port);
        for(int i = 0; i < list->optionsCount; i++)
            fprintf(file, "    %s \"%s\"\n", list->options[i].name, list->options[i].value);
        fprintf(file, "\n");
        list = list->pNext;
    }
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
 * @return 0 if string is not empty, 1 otherwise
 */
int isEmptyString(const char* str) {
    if (str == NULL)
        return 0;
    char ch = *(str++);
    while (ch != '\0') {
        if(!isspace(ch))
            return 0;
        ch = *(str++);
    } 
    return 1;
}

/**
 * Read configuration from file
 * @param fname file name
 * @return pointer to set of readed interfaces
 */
TInterface* readConfigFromFile(const char *fname) {
    FILE *file = fopen(fname, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file for read <%s>\n", fname);
        return NULL;
    }
    // TODO: potential problem if line is longer
    const int lineBufSize = 128;
    char lineBuf[lineBufSize];
    int slot = -1;
    int port = -1;
    TInterface* interfaceList = NULL;
    TInterface* lastInterface = NULL;
    while (fgets(lineBuf, lineBufSize, file) != NULL) {
        if (strlen(lineBuf) >= (lineBufSize-1)) {
            fprintf(stderr, "Too long line in file\n");
            interfaceListDelete(interfaceList);
            interfaceList = NULL;
            break;
        }
        if (isEmptyString(lineBuf)) // skip empty line
            continue;
        // Check "interface ..." line
        int isInterface = strcmp(lineBuf, "interface");
        if (isInterface > 0) {
            if (sscanf(lineBuf, "interface %d/%d", &slot, &port) < 2) {
                fprintf(stderr, "Unrecognised interface line %s\n", lineBuf);
                interfaceListDelete(interfaceList);
                interfaceList = NULL;
                break;
            }
            // TODO: check if interface is already exist
            // create new interface
            TInterface* newInterface = interfaceCreate();
            if (newInterface == NULL) {
                fprintf(stderr, "readConfigFromFile internal error\n");
                interfaceListDelete(interfaceList);
                interfaceList = NULL;
                break;
            }
            if (interfaceList == NULL) {
                lastInterface = interfaceList = newInterface;
            } else {
                lastInterface = lastInterface->pNext = newInterface;
            }
            lastInterface->slot = slot;
            lastInterface->port = port;
        } else { // option line
            if (lastInterface == NULL) {
                fprintf(stderr, "Option line before interface definition\n");
                break;
            }
            char optionName[lineBufSize];
            char optionValue[lineBufSize];
            optionName[0] = optionValue[0] = '\0';
            if (sscanf(lineBuf, " %s \"%s\"", optionName, optionValue) < 2) {
                fprintf(stderr, "Unrecognised option line <%s>\n", lineBuf);
                interfaceListDelete(interfaceList);
                interfaceList = NULL;
                break;
            }
            if (strlen(optionName) >= MAX_NAME_LEN) {
                fprintf(stderr, "Option name too long <%s>\n", optionName);
                interfaceListDelete(interfaceList);
                interfaceList = NULL;
                break;
            }
            // TODO: potential error if not trialing '"' exist
            optionValue[strlen(optionValue)-1] = '\0'; // remove trialing '"'
            if (strlen(optionValue) >= MAX_VALUE_LEN) {
                fprintf(stderr, "Option value too long <%s>\n", optionValue);
                interfaceListDelete(interfaceList);
                interfaceList = NULL;
                break;
            }
            interfaceOptionSet(lastInterface, optionName, optionValue);
        }
    }
    fclose(file);
    return interfaceList;
}

/**
 * Write configuration to file
 * @param fname file name
 * @param interfaces pointer to set of readed interfaces
 */
void writeConfigToFile(const char *fname, const TInterface *interfaceList) {
    FILE *file = fopen(fname, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file for write <%s>\n", fname);
        return;
    }
    interfaceListWrite(file, interfaceList);
    fclose(file);
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
int cfgGet(const char *fName, tCommandParam *const param) {
    if((fName == NULL) || (param == NULL))
        return -1;
    
    TInterface *interfaceList = readConfigFromFile(fName);
    if (interfaceList == NULL)
        return 0;

    int result = 0;

    TInterface *interface = interfaceList;
    while (interface != NULL) {
        if ((interface->slot == param->slot) && (interface->port == param->port))
            break;
        interface = interface->pNext;
    }

    if (interface == NULL) {
        fprintf(stderr, "Error: s/p not found\n");
    } else {
        int optionIndex = 0;
        for (optionIndex = 0; optionIndex < interface->optionsCount; optionIndex++)
            if(strcmp(param->option, interface->options[optionIndex].name) == 0)
                break;
        if (optionIndex >= interface->optionsCount)
            fprintf(stderr, "Error: option not found\n");
        else {
            strcpy(param->value, interface->options[optionIndex].value);
            result = 1;
        }
    }

    interfaceListDelete(interfaceList);
    return result;
}
int cfgDel(const char *fName, const tCommandParam *const param) {
    UNUSED(fName);
    UNUSED(param);
    return 0;
}
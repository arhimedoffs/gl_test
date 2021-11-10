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


/**
 * Create new interface node and allocate memory for options
 */
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

/**
 * Free options memory and destroy interface
 */
void interfaceDelete(TInterface *interface) {
    if (interface == NULL)
        return;
    if (interface->options != NULL)
        free(interface->options);
    free(interface);
}

/**
 * Delete full interfaces list
 */
void interfaceListDelete(TInterface *list) {
    while (list != NULL) {
        TInterface *next = list->pNext;
        interfaceDelete(list);
        list = next;
    }
}

/**
 * Write interface list to text file with formatting
 * Interface without options skipped!
 */
void interfaceListWrite(FILE *file, const TInterface *list) {
    if(file == NULL)
        return;
    while (list != NULL) {
        if (list->optionsCount > 0) {
            fprintf(file, "interface %d/%d\n", list->slot, list->port);
            for(int i = 0; i < list->optionsCount; i++)
                fprintf(file, "    %s \"%s\"\n", list->options[i].name, list->options[i].value);
            fprintf(file, "\n");
        }
        list = list->pNext;
    }
}

/**
 * Find option index by name
 * @retval option index if found, -1 othervise
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
    if (i >= interface->optionsCount)
        return -1;
    return i;
}

/**
 * Add if not exist or modify interface option
 * @return negative on error, modified option index otherwise
 */
// TODO: check actual string values length
int interfaceOptionSet(TInterface *interface, const char *optionName, const char *optionValue) {
    if ((interface == NULL) || (optionName == NULL) || (optionValue == NULL))
        return -1;
    if (interface->options == NULL)
        return -1;
    
    // Search if option already exist
    int optionIndex = interfaceOptionFind(interface, optionName);

    if (optionIndex >= 0) {
        // Update existing option
        strcpy(interface->options[optionIndex].value, optionValue);
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
        optionIndex = interface->optionsCount++;
        strcpy(interface->options[optionIndex].name, optionName);
        strcpy(interface->options[optionIndex].value, optionValue);
    }
    return optionIndex;
}

/**
 * Delete interface option if exist
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
 * Check if string consists only from space symbols
 * @return 0 if string is not empty, 1 otherwise
 */
static int isEmptyString(const char* str) {
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
 * Read configuration from text file
 * @param fname file name
 * @return pointer to set of readed interfaces
 */
TInterface* readConfigFromFile(FILE *file) {
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
    if((fName == NULL) || (param == NULL))
        return -1;

    TInterface *interfaceList = NULL;

    FILE *file = fopen(fName, "r");
    if (file != NULL) {
        interfaceList = readConfigFromFile(file);
        fclose(file);
    }

    int result = 0;

    TInterface *interface = interfaceList;
    while (interface != NULL) {
        if ((interface->slot == param->slot) && (interface->port == param->port))
            break;
        interface = interface->pNext;
    }

    if (interface == NULL) {
        // Add new interface
        TInterface *newInterface = interfaceCreate();
        if (newInterface == NULL) {
            fprintf(stderr, "Error: cfgAdd internal error\n");
            result = -1;
        } else {
            TInterface *lastInterface = interfaceList;
            if (interfaceList == NULL)
                lastInterface = interfaceList = newInterface;
            else {
                while(lastInterface->pNext != NULL)
                    lastInterface = lastInterface->pNext;
                lastInterface = lastInterface->pNext = newInterface;
            }
            newInterface = NULL;
            lastInterface->slot = param->slot;
            lastInterface->port = param->port;
            interfaceOptionSet(lastInterface, param->option, param->value);
            result = 1;
        }
    } else {
        interfaceOptionSet(interface, param->option, param->value);
        result = 1;
    }
    if (result >= 0)
        writeConfigToFile(fName, interfaceList);

    interfaceListDelete(interfaceList);
    return result;
}

/**
 * Get option value if found
 * @return negative on error, 0 if not found, 1 if success
 */
int cfgGet(const char *fName, tCommandParam *const param) {
    if((fName == NULL) || (param == NULL))
        return -1;
    
    TInterface *interfaceList = NULL;

    FILE *file = fopen(fName, "r");
    if (file != NULL) {
        interfaceList = readConfigFromFile(file);
        fclose(file);
    } else {
        fprintf(stderr, "Error: file <%s> not found\n", fName);
        return -1;
    }
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
        int optionIndex = interfaceOptionFind(interface, param->option);
        if ((optionIndex < 0) || (optionIndex >= interface->optionsCount))
            fprintf(stderr, "Error: option not found\n");
        else {
            strcpy(param->value, interface->options[optionIndex].value);
            result = 1;
        }
    }

    interfaceListDelete(interfaceList);
    return result;
}

/**
 * Delete interface option
 * @return -1 if error, 0 if option not found, 1 otherwise
 */ 
int cfgDel(const char *fName, const tCommandParam *const param) {
    if((fName == NULL) || (param == NULL))
        return -1;
    
    TInterface *interfaceList = NULL;

    FILE *file = fopen(fName, "r");
    if (file != NULL) {
        interfaceList = readConfigFromFile(file);
        fclose(file);
    } else {
        fprintf(stderr, "Error: file <%s> not found\n", fName);
        return -1;
    }
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
        if (interfaceOptionDel(interface, param->option) <= 0)
            fprintf(stderr, "Error: option not found\n");
        else {
            writeConfigToFile(fName, interfaceList);
            result = 1;
        }
    }

    interfaceListDelete(interfaceList);
    return result;
}
#include <stdio.h>
#include <string.h>
#include "config.h"

const char configPath[] = "sp.cfg";

const char usageInfo[] = "Demo program with configuration file\n\
Usage: gl_configurator <command> <options>\n\
\n\"add\" - create a new entry in config file or update the existing one\n\
> gl_configurator add \"<s_p>\" \"<option_name>\" \"<option_value>\"\n\
> gl_configurator add \"<s_p_range>\" \"<option_name>\" \"<option_value>\"\n\
\n\"get\" - get the value of the option\n\
> gl_configurator get \"<s_p>\" \"<option_name>\"\n\
\n\"del\" - delete the option\n\
> gl_configurator del \"<s_p>\" \"<option_name>\"\n";

void printHelp(void) {
    printf(usageInfo);
}

int parseSlotPort(const char *opt, tCommandParam *params) {
    int slot = -1;
    int port = -1;
    int slotRange = -1;
    int portRange = -1;
    int readedParams = sscanf(opt, "%d/%d-%d/%d", &slot, &port, &slotRange, &portRange);
    if (readedParams == 4) {
        params->slot = slot;
        params->port = port;
        params->slotRange = slotRange;
        params->portRange = portRange;
    } else {
        readedParams = sscanf(opt, "%d/%d", &slot, &port);
        if (readedParams == 2) {
            params->slot = slot;
            params->port = port;
            params->slotRange = -1;
            params->portRange = -1;
        } else {
            fprintf(stderr, "Unrecognised slot-port pair <%s>\n", opt);
            readedParams = -1;
        }
    }
    return readedParams;
}

int parseOptionName(const char *opt, tCommandParam *params) {
    int len = strlen(opt);
    if (len > MAX_NAME_LEN) {
        fprintf(stderr, "Name length too long\n");
        return -1;
    }
    strcpy(params->option, opt);
    return len;
}

int parseOptionValue(const char *opt, tCommandParam *params) {
    int len = strlen(opt);
    if (len > MAX_NAME_LEN) {
        fprintf(stderr, "Value length too long\n");
        return -1;
    }
    strcpy(params->value, opt);
    return len;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        printHelp();
        return 0;
    }

    int exitCode = 0;

    const char *strCommand = argv[1];
    tCommand command = CMD_EMPTY;
    tCommandParam cmdParams;
    if (! strcmp(strCommand, "add"))
        command = CMD_ADD;
    else if (! strcmp(strCommand, "get"))
        command = CMD_GET;
    else if (! strcmp(strCommand, "del"))
        command = CMD_DEL;
    else
        command = CMD_NOT_EXIST;

    switch (command) {
        case CMD_ADD:
        {
            if (argc != 5) {
                fprintf(stderr, "Incorrect number of add params\n");
                exitCode = -2;
                break;
            }
            const char *strSlotPort = argv[2];
            const char *name = argv[3];
            const char *value = argv[4];
            if (parseSlotPort(strSlotPort, &cmdParams) < 0) {
                exitCode = -2;
                break;
            }
            if (parseOptionName(name, &cmdParams) < 0) {
                exitCode = -3;
                break;
            }
            if (parseOptionValue(value, &cmdParams) < 0) {
                exitCode = -4;
                break;
            }
        }
        break;
        case CMD_GET:
        {
            if (argc != 4) {
                fprintf(stderr, "Incorrect number of get params\n");
                exitCode = -2;
                break;
            }
            char *strSlotPort = argv[2];
            const char *name = argv[3];
            if (parseSlotPort(strSlotPort, &cmdParams) < 0) {
                exitCode = -2;
                break;
            }
            if (parseOptionName(name, &cmdParams) < 0) {
                exitCode = -3;
                break;
            }
            cmdParams.value[0] = '\0';
        }
        break;
        case CMD_DEL:
        {
            if (argc != 4) {
                fprintf(stderr, "Incorrect number of del params\n");
                exitCode = -2;
                break;
            }
            char *strSlotPort = argv[2];
            const char *name = argv[3];
            if (parseSlotPort(strSlotPort, &cmdParams) < 0) {
                exitCode = -2;
                break;
            }
            if (parseOptionName(name, &cmdParams) < 0) {
                exitCode = -3;
                break;
            }
            cmdParams.value[0] = '\0';
        }
        break;
        default:
            fprintf(stderr, "Unrecognised command <%s>\n", argv[1]);
            exitCode = -1;
    }

    if (exitCode < 0)
        return exitCode;
    
    switch (command) {
        case CMD_ADD:
            if (cmdParams.slotRange < 0 && cmdParams.portRange < 0) {
                exitCode = cfgAdd(configPath, &cmdParams);
            } else if (cmdParams.slot != cmdParams.slotRange) {
                fprintf(stderr, "Slot range not implemented!\n");
                exitCode = 1;
            } else if (cmdParams.port > cmdParams.portRange) {
                fprintf(stderr, "Incorrect port range!\n");
                exitCode = 1;
            } else {
                for (int i = cmdParams.port; i <= cmdParams.portRange; i++) {
                    cmdParams.port = i;
                    exitCode = cfgAdd(configPath, &cmdParams);
                }
            }
            if (exitCode > 0)
                exitCode = 0;
            break;
        case CMD_GET:
            exitCode = cfgGet(configPath, &cmdParams);
            if (exitCode > 0) {
                printf("\"%s\"\n", cmdParams.value);
                exitCode = 0;
            }
            break;
        case CMD_DEL:
            exitCode = cfgDel(configPath, &cmdParams);
            break;
        default:
            fprintf(stderr, "Command not implemented\n");
    }

    return exitCode;
}
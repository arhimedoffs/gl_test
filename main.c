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
            readedParams = -1;
        }
    }
    return readedParams;
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
            char *strSlotPort = argv[2];
            int params = parseSlotPort(strSlotPort, &cmdParams);
            if (params < 0) {
                fprintf(stderr, "Unrecognised slot-port pair <%s>", strSlotPort);
                exitCode = -2;
            }
        }
        break;
        case CMD_GET:
        {
            char *strSlotPort = argv[2];
            int params = parseSlotPort(strSlotPort, &cmdParams);
            if (params < 0) {
                fprintf(stderr, "Unrecognised slot-port pair <%s>", strSlotPort);
                exitCode = -2;
            }
        }
        break;
        case CMD_DEL:
        {
            char *strSlotPort = argv[2];
            int params = parseSlotPort(strSlotPort, &cmdParams);
            if (params < 0) {
                fprintf(stderr, "Unrecognised slot-port pair <%s>", strSlotPort);
                exitCode = -2;
            }
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
            exitCode = cfgAdd(configPath, &cmdParams);
            break;
        case CMD_GET:
            exitCode = cfgGet(configPath, &cmdParams);
            break;
        case CMD_DEL:
            exitCode = cfgDel(configPath, &cmdParams);
            break;
        default:
            fprintf(stderr, "Command not implemented\n");
    }

    return exitCode;
}
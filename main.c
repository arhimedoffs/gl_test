#include <stdio.h>
#include "config.h"

const char configPath[] = "sp.cfg";

const char usageInfo[] = "Demo program with configuration file\n\
Usage:\n\
\"add\" - create a new entry in config file or update the existing one\n\
> ./gl_configurator add \"<s_p>\" \"<option_name>\" \"<option_value>\"\n\
> ./gl_configurator add \"<s_p_range>\" \"<option_name>\" \"<option_value>\"\n\
\"get\" - get the value of the option\n\
> ./gl_configurator get \"<s_p>\" \"<option_name>\"\n\
\"del\" - delete the option\n\
> ./gl_configurator del \"<s_p>\" \"<option_name>\"\n";

void printHelp(void) {
    printf(usageInfo);
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        printHelp();
        return 0;
    }
    return 0;
}
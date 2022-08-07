
#include <getopt.h>
#include <string>

#include "PDPSettings.hpp"

PDPSettings parseSettings(int argc, char** argv) {
    opterr = true;

    PDPSettings settings;

    // Flags:
    //   --debug: enables Debugging Mode
    //   --sense1 through --sense6: enables the specified sense switch
    //   --mem <M>: sets the amount of available memory
    //     Available settings:
    //       "1x" / "4K" / "4096":    4096 words
    //       "2x" / "8K" / "8192":    8192 words
    //       "4x" / "16K" / "16384":  16384 words
    //       "8x" / "32K" / "32768":  32768 words

    option long_options[] = {
        {"debug", no_argument, nullptr, 'd'},
        {"sense1", no_argument, nullptr, '1'},
        {"sense2", no_argument, nullptr, '2'},
        {"sense3", no_argument, nullptr, '3'},
        {"sense4", no_argument, nullptr, '4'},
        {"sense5", no_argument, nullptr, '5'},
        {"sense6", no_argument, nullptr, '6'},
        {"mem", required_argument, nullptr, 'm'},
        {nullptr, 0, nullptr, 0}
    };

    int c;
    while ((c = getopt_long(argc, argv, "d1:2:3:4:5:6:m:", long_options, nullptr)) != -1) {
        switch (c) {
            case 'd':
                settings.debug = true;
                break;
            case '1':
                settings.senseSwitches[0] = true;
                break;
            case '2':
                settings.senseSwitches[1] = true;
                break;
            case '3':
                settings.senseSwitches[2] = true;
                break;
            case '4':
                settings.senseSwitches[3] = true;
                break;
            case '5':
                settings.senseSwitches[4] = true;
                break;
            case '6':
                settings.senseSwitches[5] = true;
                break;
            case 'm':
                {
                    std::string arg = optarg;
                    if (arg == "1x" || arg == "4K" || arg == "4096") {
                        settings.memory_size = 4096;
                    }
                    else if (arg == "2x" || arg == "8K" || arg == "8192") {
                        settings.memory_size = 8192;
                    }
                    else if (arg == "4x" || arg == "16K" || arg == "16384") {
                        settings.memory_size = 16384;
                    }
                    else if (arg == "8x" || arg == "32K" || arg == "32768") {
                        settings.memory_size = 32768;
                    }
                    else {
                        exit(1);
                    }
                    break;
                }
            case '?':
                break;
            default:
                exit(1);
        }
    }

    settings.tapeFile = argv[argc - 1];

    return settings;
}


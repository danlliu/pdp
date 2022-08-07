
//
// PDP-1 Simulator
// A simulator for the PDP-1
//
// Simulator CLI
// ./simulator [--debug] [FLAGS] TAPEFILE
//
// Reads in punched tape in ASCII art format (as created by the assembler).
//
// Flags:
//   --debug: enables Debugging Mode
//   --sense1 through --sense6: enables the specified sense switch
//   --mem <M>: sets the amount of available memory
//     Available settings:
//       "1x" / "4K" / "4096":    4096 words
//       "2x" / "8K" / "8192":    8192 words
//       "4x" / "16K" / "16384":  16384 words
//       "8x" / "32K" / "32768":  32768 words
//
// Debugging Mode:
//

#include "TapeReader.hpp"
#include "PDPSettings.hpp"
#include "PDPState.hpp"

int main(int argc, char** argv) {
    PDPSettings settings = parseSettings(argc, argv);

    PDPProcessor proc(settings);
    while (proc.step());
    return 0;
}



//
// PDP-1 Assembler
// Punched Tape Writer
//

#include <bitset>
#include <iostream>
#include <string>
#include <vector>

class TapeWriter {

private:

    std::ostream& os;

public:

    TapeWriter(std::ostream &os);

    void writeWord(const std::bitset<18> &word, const std::string& annotation);

};


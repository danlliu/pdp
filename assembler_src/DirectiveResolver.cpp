
#include <iostream>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "DirectiveResolver.hpp"
#include "LineParser.hpp"

std::unordered_map<std::string, PDPMacro> searchMacros(const std::vector<std::string> &lines) {
    std::regex beginMacroRegex("(([A-Za-z0-9]+): )?[ \t\r]*\\.macro[ \t\r]+([A-Za-z0-9]+)(([ \t\r]+[A-Za-z0-9]+)*)");
    std::smatch beginMacroMatch;
    std::regex endMacroRegex("[ \t\r]*\\.endmacro");

    std::unordered_map<std::string, PDPMacro> results;
    bool inMacro = false;
    PDPMacro currentMacro;
    for (const std::string& line : lines) {
        if (inMacro && std::regex_match(line, endMacroRegex)) {
#ifdef DEBUG
            std::cout << "Found macro \"" << currentMacro.name << "\"" << std::endl;
#endif
            inMacro = false;
            results[currentMacro.name] = currentMacro;

            currentMacro = {
                "",
                {},
                {}
            };
        } else if (!inMacro && std::regex_match(line, beginMacroMatch, beginMacroRegex)) {
            inMacro = true;
            currentMacro.name = beginMacroMatch[3].str();
            currentMacro.parameters.clear();

            std::string operands = beginMacroMatch[4];

            // split operands by whitespace
            std::regex  operandRegex("[^ \t\r]+");
            for (auto it = std::sregex_iterator(begin(operands), end(operands), operandRegex); it != std::sregex_iterator(); ++it) {
                currentMacro.parameters.emplace_back(it->str());
            }

            currentMacro.contentLines.clear();
        } else if (inMacro) {
            currentMacro.contentLines.emplace_back(line);
        }
    }
    return results;
}

std::vector<std::string> resolveDirectives(const std::vector<std::string> &lines, const std::unordered_map<std::string, PDPMacro> &macros) {
    std::regex beginMacroRegex("(([A-Za-z0-9]+): )?[ \t\r]*\\.macro[ \t\r]+([A-Za-z0-9]+)(([ \t\r]+[A-Za-z0-9]+)*)");
    std::regex endMacroRegex("[ \t\r]*\\.endmacro");

    std::vector<std::string> results;
    bool inMacro = false;

    auto addLinesToMacro = [&](std::string macroName, std::vector<std::string> operands) {
        auto it = macros.find(macroName);
        if (it == end(macros)) {
            throw UndefinedMacro{macroName};
        }
        PDPMacro m = it->second;
        std::vector<std::regex> parameterRegexes;
        for (const std::string &param : m.parameters) {
            parameterRegexes.emplace_back(std::regex{param});
        }

        // go through macro body and replace parameters
        for (const std::string &line : m.contentLines) {
            std::string evaluated = line;
            for (int i = 0; i < size(parameterRegexes); ++i) {
                evaluated = std::regex_replace(evaluated, parameterRegexes[i], operands[i]);
            }
            results.emplace_back(evaluated);
        }

        return;
    };

    for (auto line : lines) {
        if (inMacro && std::regex_match(line, endMacroRegex)) {
            inMacro = false;
        } else if (!inMacro && std::regex_match(line, beginMacroRegex)) {
            inMacro = true;
        } else if (!inMacro) {
            // don't actually care about line num rn
            LineType parsed = parseLine(line, -1);
            if (std::holds_alternative<InstructionLine>(parsed)) {
                // instruction maybe
                InstructionLine il = std::get<InstructionLine>(parsed);
                if (macros.find(il.opcode) != end(macros)) {
                    // nvm it's a macro
                    std::vector<std::string> operands;
                    operands.reserve(1);

                    if (il.operand) operands.emplace_back(il.operand.value());

                    addLinesToMacro(il.opcode, operands);
                } else {
                    results.emplace_back(line);
                }
            } else if (std::holds_alternative<DirectiveLine>(parsed)) {
                // directive
                DirectiveLine dl = std::get<DirectiveLine>(parsed);
                if (dl.directive == ".") {
                    continue;
                }
                if (dl.directive == ".fill") {
                    results.emplace_back(line);
                }
                if (dl.directive == ".space") {
                    if (size(dl.operands) == 0) throw InvalidDirective{"Operand is required for .space"};
                    int numZeroes = stoi(dl.operands[0]);
                    if (numZeroes < 0) throw InvalidDirective{".space cannot take negative values"};
                    for (int i = 0; i < numZeroes; ++i) {
                        results.emplace_back(" .fill 0");
                    }
                }
                // macro is taken care of
            } else {
                // macro invocation
                MacroInvocation mi = std::get<MacroInvocation>(parsed);
                addLinesToMacro(mi.macro, mi.operands);
            }
        }
    }
    return results;
}


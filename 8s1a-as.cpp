#define err(c, v, e) std::cerr << argv[1] << ":" << linenum << ":" << c << ": error: \"" << v << retnames[e] << "\n" << line << "\n"; err |= e;
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <bitset>
#include <utility>

typedef std::bitset<8> byte;
typedef std::bitset<16> byte16;

std::string trim(const std::string& str,
                 const std::string& whitespace = " \t")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

std::string reduce(const std::string& str,
                   const std::string& fill = " ",
                   const std::string& whitespace = " \t")
{
    // trim first
    auto result = trim(str, whitespace);

    // replace sub ranges
    auto beginSpace = result.find_first_of(whitespace);
    while (beginSpace != std::string::npos)
    {
        const auto endSpace = result.find_first_not_of(whitespace, beginSpace);
        const auto range = endSpace - beginSpace;

        result.replace(beginSpace, range, fill);

        const auto newStart = beginSpace + fill.length();
        beginSpace = result.find_first_of(whitespace, newStart);
    }

    return result;
}

std::vector<std::string> split(const std::string &s, char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;
  if (s.find(delim) == std::string::npos) {
      elems.push_back(s);
      return elems;
  }
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
    // elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
  }
  return elems;
}

enum return_values {
    ERR_OK = 0,
    ERR_USAGE = 1,
    ERR_INVALID_INSTRUCTION = 2,
    ERR_NOT_A_NUMBER = 4,
    ERR_INVALID_NUMBER = 8,
    ERR_INVALID_SECTION = 16,
};

std::vector<std::string> retnames(17, "\" unknown error");

int main(int argc, const char * argv[]) {
    if (argc < 3) {
        std::cout << "usage: " << argv[0] << " <file.s> <output>\n";
        return ERR_USAGE;
    }
    retnames[ERR_INVALID_INSTRUCTION] = "\" is not a valid instruction";
    retnames[ERR_NOT_A_NUMBER] = "\" is not a number";
    retnames[ERR_INVALID_NUMBER] = "\" is not a valid 8-bit number";
    retnames[ERR_INVALID_SECTION] = "\" is not a valid section";
    //retnames.insert(retnames.begin() + ERR_, " ");
    std::ifstream in;
    in.open(argv[1]);
    std::string line;
    std::vector<char> outfile;
    std::map<std::string, std::string> replacements;
    replacements["rbx"] = "$0";
    std::map<std::string, int> newInstructions;
    int linenum;
    int section = -1;
    int err = 0;
    int spec = 0;
    int inIf = 0;
    int lastvar = 0;
    for (linenum = 1; in.good() && !in.eof(); linenum++) {
        std::getline(in, line);
        line = reduce(line);
        if (line.substr(0, 7) == "section") {
            std::string type = line.substr(8, line.find(';') - 9);
            type = reduce(type);
            if (type == ".text") section = 0;
            else if (type == ".data") section = 1;
            else if (type == ".bss") section = 2;
            else if (type == ".comment") section = -1;
            else if (type == ".spec") section = 3;
            else {
                err(8, type, ERR_INVALID_SECTION);
                section = -1;
            }
            continue;
        }
        if (section == 0) { // text
            std::string instr = line.substr(0, 3);
            int twonum = -1;
            if (instr == "add") {
                outfile.push_back(0);
            } else if (instr == "sub") {
                outfile.push_back(1);
            } else if (instr == "set") {
                outfile.push_back(2);
            } else if (instr == "ieq") {
                outfile.push_back(3);
                inIf++;
            } else if (instr == "ine") {
                outfile.push_back(4);
                inIf++;
            } else if (instr == "igt") {
                outfile.push_back(5);
                inIf++;
            } else if (instr == "ilt") {
                outfile.push_back(6);
                inIf++;
            } else if (instr == "out") {
                outfile.push_back(7);
            } else if (instr == "ldv" && spec > 1) {
                outfile.push_back(10);
                twonum = 0;
            } else if (instr == "sav" && spec > 1) {
                outfile.push_back(11);
                twonum = 1;
            } else if (instr == "jmp" && spec > 0) {
                outfile.push_back(8);
                twonum = 0;
            } else if (instr == "ret") {
                outfile.push_back(9);
                if (inIf == 0) goto write;
                continue;
            } else if (instr == "end" && inIf > 0) {
                outfile.push_back(126);
                inIf--;
                continue;
            } else if (newInstructions.find(instr) != newInstructions.end()) {
                outfile.push_back(newInstructions[instr]);
            } else {
                err(0, instr, ERR_INVALID_INSTRUCTION);
            }
            std::string argstr = line.substr(4, line.find(";") - 5);
            std::vector<std::string> args = split(reduce(argstr), ',');
            int i = 0;
            int ar = 1;
            for (std::string arg : args) {
                arg = reduce(arg);
                if (replacements.find(arg) != replacements.end()) {
                    arg = replacements[arg];
                }
                if (arg[0] == '$') {
                    arg = arg.substr(1);
                    outfile.push_back(255);
                }
                int argnum;
                try {
                    argnum = std::stoi(arg);
                } catch (const std::invalid_argument& e) {
                    err(ar, arg, ERR_NOT_A_NUMBER);
                    continue;
                }
                if (twonum != -1 && twonum == i && argnum < 65536) {
                    std::pair<byte, byte> newnum = splitByte(byte16(argnum));
                    outfile.push_back((char)std::get<0>(newnum).to_ulong());
                    argnum = (int)std::get<1>(newnum).to_ulong();
                } else {
                    i++;
                }
                if (argnum > 255) {
                    err(ar, argnum, ERR_INVALID_NUMBER);
                    continue;
                }
                outfile.push_back((char)argnum);
                ar++;
            }
        } else if (section == 1) { // data
            std::string name = line.substr(0, line.find(' ') + 1);
            std::string value = line.substr(line.find(' ') + 1, line.find(';') - line.find(' ') - 2);
            name = reduce(name);
            value = reduce(value);
            int argnum;
            try {
                argnum = std::stoi(value);
            } catch (const std::invalid_argument& e) {
                err(line.find(' ') + 1, value, ERR_NOT_A_NUMBER);
                continue;
            }
            if (argnum > 255) {
                err(line.find(' ') + 1, argnum, ERR_INVALID_NUMBER);
                continue;
            }
            replacements.insert(std::make_pair(name, value));
        } else if (section == 2) {
            std::string name = line.substr(0, line.find(' '));
            std::string value = line.substr(line.find(' ') + 1, line.find(';') - line.find(' ') - 2);
            name = reduce(name);
            value = reduce(value);
            std::string curvar;
            if (value != "") {
                std::vector<std::string> values = split(value, ',');
                values[0] = reduce(values[0]);
                int argnum;
                try {
                    argnum = std::stoi(values[0]);
                } catch (const std::invalid_argument& e) {
                    err(line.find(' ') + 1, values[0], ERR_NOT_A_NUMBER);
                    continue;
                }
                lastvar = argnum;
                curvar = std::to_string(argnum);
            } else curvar = std::to_string(++lastvar);
            if (lastvar > 255) {
                err(line.find(' ') + 1, curvar, ERR_INVALID_NUMBER);
                continue;
            }
            replacements.insert(std::make_pair(name, "$" + curvar));
            if (values.size() > 1) {
                values[1] = reduce(values[1]);
                int newargnum;
                try {
                    newargnum = std::stoi(values[1]);
                } catch (const std::invalid_argument& e) {
                    err(line.find(',') + 1, values[1], ERR_NOT_A_NUMBER);
                    continue;
                }
                if (newargnum > 255) {
                    err(line.find(',') + 1, newargnum, ERR_INVALID_NUMBER);
                    continue;
                }
                outfile.push_back(2);
                outfile.push_back((char)lastvar);
                outfile.push_back((char)newargnum);
            }
        } else if (section == 3) {
            std::string inst = line.substr(0, 4);
            if (inst == "spec") {
                std::string value = reduce(line.substr(4, line.find(';') - 5));
                if (value == "8S1A") spec = 0;
                else if (value == "ROM") spec = 1;
                else if (value == "8H1A") spec = 2;
                else {
                    err(5, value, ERR_INVALID_INSTRUCTION);
                }
            } else if (inst == "inst") {
                std::string argstr = line.substr(4, line.find(";") - 5);
                std::vector<std::string> args = split(reduce(argstr), ',');
                if (args.size() < 2) {
                    err(5, argstr, ERR_INVALID_INSTRUCTION);
                } else newInstructions[args[0]] = std::stoi(args[1]);
            } else {
                err(0, inst, ERR_INVALID_INSTRUCTION);
            }
        }
    }
write:
    if (err != ERR_OK) {
        return err;
    }
    if (outfile[outfile.size() - 1] != 11) {
        std::cout << argv[1] << ":" << linenum + 1 << ":0: warning: missing 'ret' at end of program\n";
        outfile.push_back(11);
    }
    in.close();
    std::ofstream out;
    out.open(argv[2], std::ios_base::binary | std::ios_base::out);
    out.write(&outfile[0], outfile.size());
    out.close();
    return 0;
}

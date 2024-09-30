/*
Hack Assembler

This is an awesome assembler for the Nand to Tetris course
author: wang yue
email: wy_workplace@163.com
date: 2024-09-30
version: 1.0
This is a C++ implementation of the Hack Assembler. It reads a file containing
assembly code and generates a binary file that can be loaded into the Hack
computer. The assembler supports the following instructions:

- A instruction: @address
- C instruction: dest=comp;jump
- L instruction: (address)

The assembler translates each instruction into a binary string of 16 bits. The
first 15 bits represent the binary code of the instruction, and the last bit
represents the valid bit. The valid bit is set to 1 for valid instructions and
0 for invalid instructions.

The assembler uses the following tables to translate the assembly code into binary
strings:

- dest table: maps the destination field to a binary string
- comp table: maps the computation field to a binary string
- jump table: maps the jump field to a binary string

The assembler also uses a symbol table to store the addresses of symbols. The
symbol table is implemented as an unordered_map, which allows for constant time
access to the address of a symbol.

To use the assembler, you can run the following command:

./HackAssembler input.asm input.hack

This will assemble the input.asm file and generate a binary file named
"input.hack".
The assembler can also be used as a standalone program by running the
"assembler.exe" file.
The source code is available on GitHub: https://github.com/wynagito/HackAssembler 

*/

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;

#define A_INSTRUCTION 1
#define C_INSTRUCTION 2
#define L_INSTRUCTION 3

void trim(string &s);
bool AllisNum(string s);
int stonum(string str);

class Code
{
public:
    unordered_map<string, string> destMap;
    unordered_map<string, string> compMap;
    unordered_map<string, string> jumpMap;
    Code()
    {
        destMap["null"] = "000";
        destMap["M"] = "001";
        destMap["D"] = "010";
        destMap["DM"] = "011";
        destMap["MD"] = "011";
        destMap["A"] = "100";
        destMap["AM"] = "101";
        destMap["AD"] = "110";
        destMap["ADM"] = "111";
        compMap["0"] = "101010";
        compMap["1"] = "111111";
        compMap["-1"] = "111010";
        compMap["D"] = "001100";
        compMap["A"] = "110000";
        compMap["M"] = "110000";
        compMap["!D"] = "001101";
        compMap["!A"] = "110001";
        compMap["!M"] = "110001";
        compMap["-D"] = "001111";
        compMap["-A"] = "110011";
        compMap["-M"] = "110011";
        compMap["D+1"] = "011111";
        compMap["A+1"] = "110111";
        compMap["M+1"] = "110111";
        compMap["D-1"] = "001110";
        compMap["A-1"] = "110010";
        compMap["M-1"] = "110010";
        compMap["D+A"] = "000010";
        compMap["D+M"] = "000010";
        compMap["D-A"] = "010011";
        compMap["D-M"] = "010011";
        compMap["A-D"] = "000111";
        compMap["M-D"] = "000111";
        compMap["D&A"] = "000000";
        compMap["D&M"] = "000000";
        compMap["D|A"] = "010101";
        compMap["D|M"] = "010101";
        jumpMap["null"] = "000";
        jumpMap["JGT"] = "001";
        jumpMap["JEQ"] = "010";
        jumpMap["JGE"] = "011";
        jumpMap["JLT"] = "100";
        jumpMap["JNE"] = "101";
        jumpMap["JLE"] = "110";
        jumpMap["JMP"] = "111";
    }
    string dest(string d)
    {
        return destMap[d];
    }
    string comp(string c)
    {
        return c.find('M') == -1 ? "0" + compMap[c] : "1" + compMap[c];
    }
    string jump(string j)
    {
        return jumpMap[j];
    }
};

class SymbolTable
{
public:
    unordered_map<string, int> symbolMap;
    SymbolTable()
    {
        symbolMap["SP"] = 0;
        symbolMap["LCL"] = 1;
        symbolMap["ARG"] = 2;
        symbolMap["THIS"] = 3;
        symbolMap["THAT"] = 4;
        symbolMap["R0"] = 0;
        symbolMap["R1"] = 1;
        symbolMap["R2"] = 2;
        symbolMap["R3"] = 3;
        symbolMap["R4"] = 4;
        symbolMap["R5"] = 5;
        symbolMap["R6"] = 6;
        symbolMap["R7"] = 7;
        symbolMap["R8"] = 8;
        symbolMap["R9"] = 9;
        symbolMap["R10"] = 10;
        symbolMap["R11"] = 11;
        symbolMap["R12"] = 12;
        symbolMap["R13"] = 13;
        symbolMap["R14"] = 14;
        symbolMap["R15"] = 15;
        symbolMap["SCREEN"] = 16384;
        symbolMap["KBD"] = 24576;
    }

    bool contains(string s)
    {
        return symbolMap.find(s) != symbolMap.end();
    }
    int getAddress(string s)
    {
        return symbolMap[s];
    }
    void addEntry(string s, int a)
    {
        symbolMap[s] = a;
    }
};

class Parser
{
public:
    ifstream inputFile;
    string line;

    Parser(string inputFileName)
    {
        inputFile.open(inputFileName);
    }
    ~Parser()
    {
        inputFile.close();
    }

    // check if there are more lines to read
    bool hasMoreLines()
    {
        return !inputFile.eof();
    }

    // gets the next line from the input file and make it the current line
    void advance()
    {
        getline(inputFile, line);
        // remove whitespace
        trim(line);
    }

    // returns the type of the current instruction
    int instructionType()
    {
        if (line.find('@') != -1)
            return A_INSTRUCTION;
        else if (line.find('(') != -1 && line.find(')') != -1)
            return L_INSTRUCTION;
        else
            return C_INSTRUCTION;
    }

    // returns the symbol of the current instruction
    // @xxx (xxx)
    string symbol()
    {
        if (line.find('@') != -1)
        {
            return line.substr(line.find('@') + 1);
        }
        return line.substr(line.find('(') + 1, line.find(')') - 1);
    }
    // dest = comp;jump
    // comp;jump
    // comp
    string dest()
    {
        return line.find('=') == -1 ? "null" : line.substr(0, line.find('='));
    }
    string comp()
    {
        if (line.find('=') == -1)
        {
            return line.find(';') == -1 ? line : line.substr(0, line.find(';'));
        }
        return line.find(';') == -1 ? line.substr(line.find('=') + 1) : line.substr(line.find('=') + 1, line.find(';') - line.find('=') - 1);
    }
    string jump()
    {
        return line.find(';') == -1 ? "null" : line.substr(line.find(';') + 1);
    }
};

// removes all whitespace from the string
void trim(string &s)
{
    int index = 0;
    if (!s.empty())
    {
        while ((index = s.find(' ', index)) != -1)
        {
            s.erase(index, 1);
        }
    }
}

// check if all characters in the string are numbers
bool AllisNum(string str)
{
    for (int i = 0; i < str.size(); i++)
    {
        int tmp = (int)str[i];
        if (tmp >= 48 && tmp <= 57)
        {
            continue;
        }
        else
        {
            return false;
        }
    }
    return true;
}

// convert a string to a number
// return 0 if the string is out of range
int stonum(string str)
{
    int n = str.size();
    if (n > 5)
        return 0; // overflow
    int num = 0;
    for (int i = 0; i < n; i++)
    {
        num = num * 10 + (int)str[i] - 48;
        if (num > 32767)
            return 0; // overflow
    }
    return num;
}

int main(int argc, char *argv[])
{
    string inputFileName = argv[1]; // input file name
    string outputFileName = argv[2]; // output file name
    ofstream outputFile(outputFileName);
    Parser *parser = new Parser(inputFileName);
    SymbolTable *symbolTable = new SymbolTable();
    Code *code = new Code();

    // initialize label address
    int address = 0; // next instruction address
    while (parser->hasMoreLines())
    {
        parser->advance();
        // ignore empty lines and comments
        if (parser->line.empty() || parser->line.find("//") != -1)
            continue;
        if (parser->instructionType() == L_INSTRUCTION)
        {
            string symbol = parser->symbol();
            if (!symbolTable->contains(symbol))
            {
                symbolTable->addEntry(symbol, address);
            }
        }
        else // A_INSTRUCTION or C_INSTRUCTION
        {
            address = address + 1;
        }
    }
    delete parser;

    // initialize variable address
    parser = new Parser(inputFileName);
    address = 16; // next variable address
    while (parser->hasMoreLines())
    {
        parser->advance();
        // ignore empty lines and comments
        if (parser->line.empty() || parser->line.find("//") != -1)
            continue;
        if (parser->instructionType() == A_INSTRUCTION)
        {
            string symbol = parser->symbol();
            if (!symbolTable->contains(symbol))
            {
                // if the symbol is a number, add it to the symbol table directly
                if (AllisNum(symbol))
                {
                    symbolTable->addEntry(symbol, stonum(symbol));
                }
                else
                {
                    symbolTable->addEntry(symbol, address);
                    address = address + 1;
                }
            }
        }
    }
    delete parser;

    // generate binary code
    parser = new Parser(inputFileName);
    string binaryCode = "";
    while (parser->hasMoreLines())
    {
        // clear binary code for each instruction
        binaryCode.clear();
        parser->advance();
        // ignore empty lines and comments
        if (parser->line.empty() || parser->line.find("//") != -1)
            continue;
        if (parser->instructionType() == A_INSTRUCTION)
        {
            string symbol = parser->symbol();
            int address = symbolTable->getAddress(symbol);
            for (int i = 0; i < 15; i++)
            {
                binaryCode = to_string((address >> i) & 1) + binaryCode;
            }
            binaryCode = "0" + binaryCode;
        }
        else if (parser->instructionType() == C_INSTRUCTION)
        {
            string dest = parser->dest();
            string comp = parser->comp();
            string jump = parser->jump();
            binaryCode = "111" + code->comp(comp) + code->dest(dest) + code->jump(jump);
        }
        if (!binaryCode.empty())
        {
            outputFile << binaryCode << endl; // write binary code to file
            outputFile.flush();               // write to file immediately
        }
    }

    // close file and delete objects
    delete parser;
    delete symbolTable;
    delete code;
    outputFile.flush();
    outputFile.close();
    return 0;
}
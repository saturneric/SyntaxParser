//
// Created by Administrator on 2021/5/1.
//

#ifndef SYNTAXPARSER_AUTOMATA_H
#define SYNTAXPARSER_AUTOMATA_H

#include <map>
#include <utility>
#include <vector>
#include <string>
#include <fstream>
#include <codecvt>
#include <sstream>

class Automata {
public:

    std::wifstream input;

    explicit Automata(const std::string& path) : input(path, std::ios::binary) {
        auto* codeCvtToUTF8= new std::codecvt_utf8<wchar_t>;
        input.imbue(std::locale(input.getloc(), codeCvtToUTF8));
    }

    ~Automata() {
        input.close();
    }

    void parse() {
        while (!ifeof) {
            tokens.push_back(nextToken());
        }
    }

    void output();

private:

    using TokenType = enum {
        /* Reserve Words */
        STRUCT, BOOLEAN, SHORT, LONG,
        DOUBLE, FLOAT,
        INT8, INT16, INT32, INT64,
        UINT8, UINT16, UINT32, UINT64,
        CHAR,
        UNSIGNED,
        /* Special Symbols */
        OPENING_BRACE, CLOSING_BRACE,
        SEMICOLON,
        LEFT_BRACKET, RIGHT_BRACKET,
        MULT, PLUS, SUB, TILDE, SLASH,
        PERCENT, LEFT_SHIFT, RIGHT_SHIFT, AND, INSERT,
        DELIMITER, COMMA,
        /* Multicharacter Tokens */
        ID, LETTER, DIGIT, UNDERLINE, T_TRUE, T_FALSE,
        INTEGER, INTEGER_TYPE_SUFFIX, STRING, T_BOOLEAN,
        /* None & Error & EOF */
        NONE, ERROR, T_EOF
    };


    using StateType = enum {
        START,
        S_LETTER, S_UNDERLINE, S_DIGIT,
        INT_0, INT_NOT_0, INT_TYPE_SUFFIX,
        STRING_START, STRING_END,
        S_SIGN,
        DONE,
        S_NONE
    };

    using ReservedWord = struct {
        std::wstring str;
        TokenType token;
    };

    struct TokenInfo {

        const int line;
        const TokenType token;
        const std::wstring str;
        const StateType state;

        TokenInfo(const int line, const TokenType token, std::wstring str, StateType state)
            : line(line), token(token), str(std::move(str)), state(state) {}

    };

    FILE* fp = nullptr;

    bool ifeof = false;

    std::vector<TokenInfo> tokens;

    int line = 1;


    const static std::vector<ReservedWord> reservedWords;

    const static std::map<TokenType, std::wstring> tokenTypeStrMap;

    const static std::map<StateType, std::wstring> stateTypeStrMap;

    wchar_t Automata::nextChar();

    void pushBackChar();

    TokenInfo nextToken();

    static TokenType reservedLookup(const std::wstring& s);



};


#endif //SYNTAXPARSER_AUTOMATA_H

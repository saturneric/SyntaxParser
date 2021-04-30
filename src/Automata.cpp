//
// Created by Administrator on 2021/5/1.
//

#include "Automata.h"

const std::vector<Automata::ReservedWord> Automata::reservedWords = {
        {L"struct", STRUCT},
        {L"boolean", BOOLEAN},
        {L"short", SHORT},
        {L"long", LONG},
        {L"double", DOUBLE},
        {L"float", FLOAT},
        {L"int8", INT8},
        {L"int16", INT16},
        {L"int32", INT32},
        {L"int64", INT64},
        {L"uint8", UINT8},
        {L"uint16", UINT16},
        {L"uint32", UINT32},
        {L"uint64", UINT64},
        {L"char", CHAR},
        {L"unsigned", UNSIGNED},
        {L"TRUE", T_TRUE},
        {L"FALSE", T_FALSE}
};

const std::map<Automata::StateType, std::wstring> Automata::stateTypeStrMap = {
        {START, L"START"},
        {S_LETTER, L"LETTER"},
        {S_UNDERLINE, L"UNDERLINE"},
        {S_DIGIT, L"DIGIT"},
        {INT_0, L"INT_0"},
        {INT_NOT_0, L"INT_NOT_0"},
        {INT_TYPE_SUFFIX, L"INT_TYPE_SUFFIX"},
        {STRING_START, L"STRING_START"},
        {STRING_END, L"STRING_END"},
        {DONE, L"DONE"},
        {S_SIGN, L"SIGN"}
};

const std::map<Automata::TokenType, std::wstring> Automata::tokenTypeStrMap = {
        {STRUCT, L"STRUCT"},
        {BOOLEAN, L"BOOLEAN"},
        {SHORT, L"SHORT"},
        {LONG, L"LONG"},
        {FLOAT, L"FLOAT"},
        {DOUBLE, L"DOUBLE"},
        {INT8, L"INT8"},
        {INT16, L"INT16"},
        {INT32, L"INT32"},
        {INT64, L"INT64"},
        {UINT8, L"UINT8"},
        {UINT16, L"UINT16"},
        {UINT32, L"UINT32"},
        {UINT64, L"UINT64"},
        {CHAR, L"CHAR"},
        {UNSIGNED, L"UNSIGNED"},
        {OPENING_BRACE, L"OPENING_BRACE"},
        {CLOSING_BRACE, L"CLOSING_BRACE"},
        {SEMICOLON, L"SEMICOLON"},
        {LEFT_BRACKET, L"LEFT_BRACKET"},
        {RIGHT_BRACKET, L"RIGHT_BRACKET"},
        {MULT, L"MULT"},
        {PLUS, L"PLUS"},
        {SUB, L"SUB"},
        {TILDE, L"TILDE"},
        {SLASH, L"SLASH"},
        {PERCENT, L"PERCENT"},
        {LEFT_SHIFT, L"LEFT_SHIFT"},
        {RIGHT_SHIFT, L"RIGHT_SHIFT"},
        {AND, L"AND"},
        {INSERT, L"INSERT"},
        {DELIMITER, L"DELIMITER"},
        {COMMA, L"COMMA"},
        {ID, L"ID"},
        {LETTER, L"LETTER"},
        {DIGIT, L"DIGIT"},
        {UNDERLINE, L"UNDERLINE"},
        {T_TRUE, L"TRUE"},
        {T_FALSE, L"FALSE"},
        {INTEGER, L"INTEGER"},
        {INTEGER_TYPE_SUFFIX, L"INTEGER_TYPE_SUFFIX"},
        {T_BOOLEAN, L"BOOLEAN"},
        {STRING, L"STRING"},
        {COMMA, L"COMMA"},
        {NONE, L"NONE"},
        {ERROR, L"ERROR"},
        {T_EOF, L"EOF"}
};

void Automata::output() {

    std::wofstream stream(L"./tokenOut.txt", std::ios::binary | std::ios::trunc);
    int temp_line = 1;
    stream << "1 ";

    for (const auto& token : tokens) {
        if (token.line > temp_line) {
            temp_line = token.line;
            stream << std::endl << temp_line << ' ';
        }

        if (token.token == ERROR) {
            stream << tokenTypeStrMap.find(token.token)->second << '{' << token.str << ", " << stateTypeStrMap.find(token.state)->second << '}' << ' ';
        }
        else {
            stream << tokenTypeStrMap.find(token.token)->second << '(' << token.str << ')' << ' ';
        }

    }

    stream.close();
}

wchar_t Automata::nextChar() {
    wchar_t c;
    input >> std::noskipws >> c;
    if (input.eof()) {
        this->ifeof = true;
    }
    return c;
}

Automata::TokenInfo Automata::nextToken() {
    TokenType currentToken = NONE;
    StateType state = START, last_state = S_NONE;
    std::wstringstream ss;
    while (state != DONE) {

        last_state = state;

        wchar_t CH = nextChar();
        bool save = true;
        if (ifeof) {
            currentToken = T_EOF;
            break;
        }
        switch (state) {
            case START:
                if (isdigit(CH)) {
                    if (CH == '0') {
                        state = INT_0;
                    }
                    else {
                        state = INT_NOT_0;
                    }
                }
                else if (isalpha(CH)) {
                    state = S_LETTER;
                }
                else if (CH == '\"') {
                    state = STRING_START;
                }
                else if ((CH == ' ') || (CH == '\t') || (CH == '\r')) {
                    save = false;
                }
                else if (CH == '\n') {
                    this->line++;
                    save = false;
                }
                else {

                    state = S_SIGN;
                    switch (CH) {
                        case '{':
                            currentToken = OPENING_BRACE;
                            break;
                        case '}':
                            currentToken = CLOSING_BRACE;
                            break;
                        case ';':
                            currentToken = SEMICOLON;
                            break;
                        case '[':
                            currentToken = LEFT_BRACKET;
                            break;
                        case ']':
                            currentToken = RIGHT_BRACKET;
                            break;
                        case '*':
                            currentToken = MULT;
                            break;
                        case '+':
                            currentToken = PLUS;
                            break;
                        case '-':
                            currentToken = SUB;
                            break;
                        case '~':
                            currentToken = TILDE;
                            break;
                        case '/':
                            currentToken = SLASH;
                            break;
                        case '%':
                            currentToken = PERCENT;
                            break;
                        case '>':
                            CH = nextChar();
                            if (CH == '>') {
                                currentToken = RIGHT_SHIFT;
                                ss << CH;
                            }
                            else {
                                pushBackChar();
                                currentToken = ERROR;
                            }
                            break;
                        case '<':
                            CH = nextChar();
                            if (CH == '<') {
                                currentToken = LEFT_SHIFT;
                                ss << CH;
                            }
                            else {
                                pushBackChar();
                                currentToken = ERROR;
                            }
                            break;
                        case '&':
                            currentToken = AND;
                            break;
                        case '^':
                            currentToken = INSERT;
                            break;
                        case '|':
                            currentToken = DELIMITER;
                            break;
                        case ',':
                            currentToken = COMMA;
                            break;
                        default:
                            currentToken = ERROR;
                            break;
                    }
                }
                break;

            case S_LETTER:
                if (CH == '_') {
                    state = S_UNDERLINE;
                }
                else if (isdigit(CH)) {
                    state = S_DIGIT;
                }
                else if (isalpha(CH)) {
                    state = S_LETTER;
                }
                else {
                    currentToken = ID;
                    pushBackChar();
                    state = DONE;
                    save = false;
                }
                break;

            case S_DIGIT:
                if (isalpha(CH)) {
                    state = S_LETTER;
                } else if (isdigit(CH)) {
                    state = S_DIGIT;
                }
                else {
                    currentToken = ID;
                    pushBackChar();
                    state = DONE;
                    save = false;
                }
                break;

            case S_UNDERLINE:
                if (isdigit(CH)) {
                    state = S_DIGIT;
                } else if (isalpha(CH)) {
                    state = S_LETTER;
                }
                else {
                    pushBackChar();
                    currentToken = ERROR;
                    save = false;
                }
                break;

            case INT_0:
                if (CH == 'l' || CH == 'L') {
                    state = INT_TYPE_SUFFIX;
                }
                else {
                    currentToken = INTEGER;
                    pushBackChar();
                    state = DONE;
                    save = false;
                }
                break;

            case INT_NOT_0:
                if (CH == 'l' || CH == 'L') {
                    state = INT_TYPE_SUFFIX;
                }
                else if (isdigit(CH)) {
                    state = INT_NOT_0;
                }
                else {
                    currentToken = INTEGER;
                    pushBackChar();
                    state = DONE;
                    save = false;
                }
                break;

            case INT_TYPE_SUFFIX:
                state = DONE;
                currentToken = INTEGER;
                pushBackChar();
                save = false;
                break;

            case STRING_START:
                if (CH == '\\') {
                    wchar_t buff_c = CH;
                    CH = nextChar();

                    if ((CH == 'b') || (CH == 't') || (CH == 'n') || (CH == 'f') || (CH == 'r')
                        || (CH == '"') || (CH == '\\')) {
                        ss << buff_c;
                        state = STRING_START;
                    }
                    else {
                        pushBackChar();
                        currentToken = ERROR;
                    }
                }
                else if (CH == '\"') {
                    state = STRING_END;
                }
                else if (CH == ' ') {
                    ss << '\\' << '4';
                    CH = '0';
                    state = STRING_START;
                }
                else {
                    state = STRING_START;
                }
                break;

            case STRING_END:
                state = DONE;
                currentToken = STRING;
                pushBackChar();
                save = false;
                break;

            case S_SIGN:
                state = DONE;
                pushBackChar();
                save = false;
                break;
        }

        if (save) {
            ss << CH;
        }

        if (state == DONE) {
            const std::wstring token = ss.str();
            if (currentToken == ID) {
                currentToken = reservedLookup(token);
            }
        }

    }

    return TokenInfo(this->line, currentToken, ss.str(), last_state);
}

Automata::TokenType Automata::reservedLookup(const std::wstring &s) {
    for (const auto& word : reservedWords)
        if (word.str == s)
            return word.token;
    return ID;
}

void Automata::pushBackChar() {
    input.seekg(-1L, std::ios::cur);
}

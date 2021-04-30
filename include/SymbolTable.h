//
// Created by Administrator on 2021/4/30.
//

#ifndef SYNTAXPARSER_SYMBOLTABLE_H
#define SYNTAXPARSER_SYMBOLTABLE_H

#include <map>
#include <vector>
#include <string>
#include <stdexcept>

#include <Symbol.h>


class SymbolTable {

    int index = 1;

    std::map<std::wstring, Symbol *> table;

    std::map<int, Symbol *> cache;

    std::vector<const Symbol *> line;

public:

    SymbolTable();

    [[nodiscard]] const std::vector<const Symbol *> &getAllSymbols() const {
        return line;
    }

    int addSymbol(const std::wstring& name, bool terminator);

    [[nodiscard]] const Symbol *getSymbol(int symbol_index) const;

    [[nodiscard]] int getSymbolIndex(const std::wstring &name) const;

    void modifySymbol(int idx, const std::wstring &name, bool terminator, bool start);

    [[nodiscard]] const Symbol *getStartSymbol() const;

};


#endif //SYNTAXPARSER_SYMBOLTABLE_H

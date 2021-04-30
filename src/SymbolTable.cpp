//
// Created by Administrator on 2021/4/30.
//

#include "SymbolTable.h"

SymbolTable::SymbolTable() {

    auto symbol = new Symbol(0, L"¦Å", true, false);
    table.insert(std::pair<std::wstring, Symbol *>(L"¦Å", symbol));
    cache.insert(std::pair<int, Symbol *>(0, symbol));
    line.push_back(symbol);

    symbol = new Symbol(-1, L"$", true, false);
    table.insert(std::pair<std::wstring, Symbol *>(L"$", symbol));
    cache.insert(std::pair<int, Symbol *>(-1, symbol));
    line.push_back(symbol);
}

int SymbolTable::addSymbol(const std::wstring &name, bool terminator) {

    Symbol *symbol = nullptr;

    if(name == L"¦Å") {
        return 0;
    } else if (name[0] == L'@') {
        symbol = new Symbol(index, name, terminator, true);
    } else {
        symbol = new Symbol(index, name, terminator, false);
    }

    const auto &it = table.find(name);
    if (it != table.end()) {
        return it->second->index;
    }
    table.insert(std::pair<std::wstring, Symbol *>(symbol->name, symbol));
    cache.insert(std::pair<int, Symbol *>(symbol->index, symbol));
    line.push_back(symbol);

    index++;

    return symbol->index;
}

const Symbol *SymbolTable::getSymbol(int symbol_index) const {
    const auto &it = cache.find(symbol_index);
    if(it != cache.end()) {
        return it->second;
    } else {
        throw std::runtime_error("symbol " + std::to_string(symbol_index) + " NOT Found");
    }
}

int SymbolTable::getSymbolIndex(const std::wstring &name) const {
    const auto &it = table.find(name);
    if(it != table.end()) {
        return it->second->index;
    } else {
        throw std::runtime_error("symbol NOT Found");
    }
}

void SymbolTable::modifySymbol(int idx, const std::wstring &name, bool terminator, bool start) {
    auto it = cache.find(idx);
    if(it != cache.end()) {
        auto p_sym = it->second;
        p_sym->name = name;
        p_sym->terminator = terminator;
        p_sym->start = start;
    }
}

const Symbol *SymbolTable::getStartSymbol() const {
    for(const auto & symbol : getAllSymbols()) {
        if(symbol->start) {
            return symbol;
        }
    }

    throw std::runtime_error("start symbol NOT Found");
}

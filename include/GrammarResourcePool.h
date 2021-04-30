//
// Created by Administrator on 2021/4/30.
//

#ifndef SYNTAXPARSER_GRAMMARRESOURCEPOOL_H
#define SYNTAXPARSER_GRAMMARRESOURCEPOOL_H


#include <set>
#include <sstream>
#include <iostream>

#include <SymbolTable.h>
#include <Production.h>

// 语法资源池
class GrammarResourcePool {

    int pdt_index = 0;

    // 符号表
    SymbolTable symbolTable;

    // 产生式
    std::vector<const Production *> productions;

    // FIRST结果存储表
    std::map<int, const std::set<int> *> firsts;

    // FOLLOW结果存储表
    std::map<int, std::set<int> *> follows;

    // 去掉首尾空格
    static std::wstring& trim(std::wstring &&str) {
        if (str.empty()) {
            return str;
        }

        str.erase(0,str.find_first_not_of(' '));
        str.erase(str.find_last_not_of(' ') + 1);
        return str;
    }

public:

    const std::set<int > *FIRST(const std::vector<int> &symbols, int start_index);

    const std::set<int>* FIRST(int symbol);

    const std::set<int> *FOLLOW(int symbol);

    void FOLLOW();

    std::set<int>* get_follow_set(int symbol);


    void print_symbols(const std::set<int> &symbols_index);

    void parse_production_string_line(const std::wstring &temp_line);

    [[nodiscard]] const std::vector<const Production *> &get_productions() const {
        return productions;
    }

    [[nodiscard]] const Symbol *getSymbol(int symbol_index) const {
        return symbolTable.getSymbol(symbol_index);
    }

    [[nodiscard]] const Symbol *getStartSymbol() const {
        return symbolTable.getStartSymbol();
    }

    int addSymbol(const std::wstring &name, bool terminator) {
        return symbolTable.addSymbol(name, terminator);
    }

    const Production *addProduction(int left, std::initializer_list<int> right);

    [[nodiscard]] const std::vector<const Symbol *> &getAllSymbols() const {
        return symbolTable.getAllSymbols();
    }

    void modifySymbol(int index, const std::wstring &name, bool terminator, bool start) {
        symbolTable.modifySymbol(index, name, terminator, start);
    }

    [[nodiscard]] int getSymbolIndex(const std::wstring &name) const {
        return symbolTable.getSymbolIndex(name);
    }
};


#endif //SYNTAXPARSER_GRAMMARRESOURCEPOOL_H

#include <iostream>
#include <ctime>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <codecvt>
#include <set>
#include <map>
#include <locale>

using namespace std;

using std::vector;
using std::wstring;
using std::wstringstream;
using std::pair;
using std::wcout;
using std::endl;
using std::to_string;

struct Symbol {

    const int index;
    const wstring name;
    const bool terminator;
    const bool start;

    Symbol(int index, wstring name, bool terminator, bool start):
    index(index),
    name(std::move(name)),
    terminator(terminator),
    start(start)
    {}

};

class SymbolTable {
    int index = 1;

    map<wstring, Symbol *> table;

    map<int, Symbol *> cache;

    vector<const Symbol *> line;

public:

    SymbolTable() {

        auto symbol = new Symbol(0, L"ε", true, false);
        table.insert(pair<wstring, Symbol *>(L"ε", symbol));
        cache.insert(pair<int, Symbol *>(0, symbol));
        line.push_back(symbol);

        symbol = new Symbol(-1, L"$", true, false);
        table.insert(pair<wstring, Symbol *>(L"$", symbol));
        cache.insert(pair<int, Symbol *>(-1, symbol));
        line.push_back(symbol);
    }

    const vector<const Symbol *> &getAllSymbols() {
        return line;
    }

    int addSymbol(const wstring& name, bool terminator) {

        Symbol *symbol = nullptr;

        if(name == L"ε") {
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
        table.insert(pair<wstring, Symbol *>(symbol->name, symbol));
        cache.insert(pair<int, Symbol *>(symbol->index, symbol));
        line.push_back(symbol);

        index++;

        return symbol->index;
    }

    const Symbol *getSymbol(int symbol_index) {
        const auto &it = cache.find(symbol_index);
        if(it != cache.end()) {
            return it->second;
        } else {
            throw runtime_error("symbol " + to_string(symbol_index) + " NOT Found");
        }
    }

    int getSymbolIndex(const wstring &name) {
        const auto &it = table.find(name);
        if(it != table.end()) {
            return it->second->index;
        } else {
            throw runtime_error("symbol NOT Found");
        }
    }

};

// 产生式
struct Production {
    const int left;
    const vector<int> right;

    Production(int left, vector<int> right): left(left), right(std::move(right)) {}

};

// 项
class Item{
    // 对应的产生式
    Production* production;
    // 点的位置
    int dotIndex = 0;
public:
    explicit Item(Production *p_pdt) {
        production = p_pdt;
    }
};

class Generator{

    // 文件输入
    wifstream input;

    // 符号表
    SymbolTable symbolTable;

    // 产生式
    vector<Production *> productions;

    // FIRST结果存储表
    map<int, set<int> *> firsts;

    // FOLLOW结果存储表
    map<int, set<int> *> follows;

    // 去掉首尾空格
    static wstring& trim(wstring &&str) {
        if (str.empty()) {
            return str;
        }

        str.erase(0,str.find_first_not_of(' '));
        str.erase(str.find_last_not_of(' ') + 1);
        return str;
    }

    set<int > *FIRST(const vector<int> &symbols, int start_index) {

        // 生成集合
        auto *non_terminator_symbols = new set<int>();

        for(int i = start_index; i < symbols.size(); i++) {

            const auto p_non_term_set = FIRST(symbols[i]);

            non_terminator_symbols->insert(p_non_term_set->begin(), p_non_term_set->end());

            const auto sec_it = p_non_term_set->find(0);
            if(sec_it != p_non_term_set->end()) {
                continue;
            } else {
                break;
            }
        }

        return non_terminator_symbols;
    }

    set<int>* FIRST(int symbol) {

        // 查找缓存
        const auto it = firsts.find(symbol);
        if(it != firsts.end()) {
            return it->second;
        }

        // 生成集合
        auto *non_terminator_symbols = new set<int>();

        // 如果是终结符
        if(symbolTable.getSymbol(symbol)->terminator) {
            non_terminator_symbols->insert(symbol);
        } else {

            bool production_found = false;

            // 遍历每一产生式
            for (const auto &production : productions) {
                Production *p_pdt = production;

                if (p_pdt->left != symbol) continue;

                production_found = true;

                for (const auto &right_symbol : p_pdt->right) {

                    const auto p_non_term_set = FIRST(right_symbol);

                    non_terminator_symbols->insert(p_non_term_set->begin(), p_non_term_set->end());

                    const auto sec_it = p_non_term_set->find(0);

                    if(sec_it != p_non_term_set->end()) {
                        continue;
                    } else {
                        break;
                    }

                }
            }

            if (!production_found) non_terminator_symbols->insert(0);
        }

        this->firsts.insert(pair<int, set<int> *>(symbol, non_terminator_symbols));

        return non_terminator_symbols;
    }

    const set<int> *FOLLOW(int symbol) {
        if(follows.empty()) {
            FOLLOW();
        }

        const auto it = follows.find(symbol);
        if(it != follows.end()) {
            return it->second;
        } else {
            throw runtime_error("symbol NOT Found");
        }
    }

    void FOLLOW() {

        for (const auto &symbol : symbolTable.getAllSymbols()) {
            if (!symbol->terminator) {
                if (symbol->start) {
                    set<int> *non_terminator_symbols = get_follow_set(symbol->index);
                    non_terminator_symbols->insert(-1);
                }
            }
        }

        // 指导没有新的符号被添加到任意FOLLOW集合
        bool ifAdded = true;

        while(ifAdded) {

            ifAdded = false;


            set<int> *non_terminator_symbols = nullptr;


            for (const auto &production : productions) {

                const auto &right_symbols = production->right;

                set<int> equal_left_non_terminators;

                for (int i = 0; i < right_symbols.size() - 1; i++) {

                    // 非终结符
                    if (!symbolTable.getSymbol(right_symbols[i])->terminator) {

                        const auto p_non_term_set = FIRST(right_symbols, i + 1);

                        // 获得FOLLOW集
                        non_terminator_symbols = get_follow_set(right_symbols[i]);

                        const size_t set_size = non_terminator_symbols->size();

                        non_terminator_symbols->insert(p_non_term_set->begin(), p_non_term_set->end());

                        // 在集合中发现空字符
                        if(non_terminator_symbols->find(0) != non_terminator_symbols->end()) {
                            non_terminator_symbols->erase(0);
                            equal_left_non_terminators.insert(right_symbols[i]);
                        }

                        // 检查是否有新的终结符号被添加
                        if(set_size < non_terminator_symbols->size()) {
                            ifAdded = true;
                        }
                    }
                }

                if(!right_symbols.empty()) {
                    if (!symbolTable.getSymbol(right_symbols[right_symbols.size() - 1])->terminator) {
                        equal_left_non_terminators.insert(right_symbols[right_symbols.size() - 1]);
                    }
                }

                for(const auto symbol : equal_left_non_terminators) {
                    // 获得左边非终结符的FOLLOW集
                    const auto left_non_terminator_symbols = get_follow_set(production->left);
                    // 获得FOLLOW集
                    non_terminator_symbols = get_follow_set(symbol);

                    const size_t set_size = non_terminator_symbols->size();

                    non_terminator_symbols->insert(
                            left_non_terminator_symbols->begin(),
                            left_non_terminator_symbols->end());

                    if(non_terminator_symbols->find(0) != non_terminator_symbols->end()) {
                        non_terminator_symbols->erase(0);
                    }

                    // 检查是否有新的终结符号被添加
                    if(set_size < non_terminator_symbols->size()) {
                        ifAdded = true;
                    }
                }

            }

        }

    }

    set<int>* get_follow_set(int symbol) {

        set<int> *non_terminator_symbols = nullptr;

        // 查找缓存
        auto it = follows.find(symbol);
        if(it != follows.end()) {
            non_terminator_symbols = it->second;
        } else {
            non_terminator_symbols = new set<int>();
            this->follows.insert(pair<int, set<int> *>(symbol, non_terminator_symbols));
        }

        return non_terminator_symbols;

    }


    void print_symbols(const set<int> &symbols_index) {
        wcout << L"{ ";
        for(const auto & symbol_index : symbols_index) {
            auto *p_sym = symbolTable.getSymbol(symbol_index);

            if(p_sym->terminator) {
                if (p_sym->name == L"ε") {
                    wcout << L" [Epsilon] ";
                }
                else wcout << L" \"" << p_sym->name << L"\" ";
            } else {
                wcout << L" " << p_sym->name << L" ";
            }

        }
        wcout << L"}" << endl;
    }

public:

    Generator(): input("syntaxInput.txt", std::ios::binary) {

        auto* codeCvtToUTF8= new std::codecvt_utf8<wchar_t>;

        input.imbue(std::locale(input.getloc(), codeCvtToUTF8));
    }

    void test() {
        auto p_non_term_set = FIRST(1);
        print_symbols(*p_non_term_set);

        p_non_term_set = FIRST(3);
        print_symbols(*p_non_term_set);

        p_non_term_set = FIRST(6);
        print_symbols(*p_non_term_set);

        FOLLOW();

        print_symbols(*FOLLOW(2));
        print_symbols(*FOLLOW(6));
        print_symbols(*FOLLOW(3));
    }

    // 得到所有的产生式
    void getProductions() {

        //读入文法文件
        wstring temp_line;

        while (getline(input, temp_line)) {
            auto middle_index = temp_line.find(L"->", 0);


            if(middle_index == string::npos) {
                throw runtime_error("-> NOT FOUND");
            }

            wstring front = trim(temp_line.substr(0, middle_index));
            int left = symbolTable.addSymbol(front, false);

            wstring back = trim(temp_line.substr(middle_index + 2, temp_line.size() - middle_index - 2));

            wstringstream terminator, non_terminator;
            vector<int> symbols;
            bool is_terminator = false;
            for(const auto &c : back) {
                if (c == L'\"') {
                    if(is_terminator) {
                        symbols.push_back(symbolTable.addSymbol(trim(terminator.str()), true));
                        terminator.str(L"");
                        terminator.clear();
                    }
                    is_terminator = !is_terminator;
                    continue;
                }
                if(c == L' ' || c == L'\r') {
                    wstring temp_symbol = trim(non_terminator.str());
                    if(!temp_symbol.empty()) {
                        symbols.push_back(symbolTable.addSymbol(trim(non_terminator.str()), false));
                        non_terminator.str(L"");
                        non_terminator.clear();
                    }
                    continue;
                }
                if(is_terminator) {
                    terminator << c;
                } else {
                    non_terminator << c;
                }
            }

            auto p_pdt = new Production(left, symbols);

            productions.push_back(p_pdt);

        }
    }

};




int main() {
    clock_t start,end;//定义clock_t变量
    start = clock(); //开始时间


    Generator generator;

    generator.getProductions();

    generator.test();
    //输出时间
    end = clock();   //结束时间
    cout << endl;
    double times = double(end-start)/CLOCKS_PER_SEC;
    cout<<"The Run time = "<<times<<"s" << " = " <<times * 1000 <<"ms" << endl;
    return 0;
}


#include <iostream>
#include <ctime>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <codecvt>
#include <set>
#include <map>
#include <functional>
#include <iomanip>
#include <algorithm>

using namespace std;

using std::vector;
using std::wstring;
using std::wstringstream;
using std::pair;
using std::wcout;
using std::endl;
using std::to_string;
using std::hash;
using std::setw;

struct Symbol {

    const int index;
    wstring name;
    bool terminator;
    bool start;

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

    [[nodiscard]] const vector<const Symbol *> &getAllSymbols() const {
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

    [[nodiscard]] const Symbol *getSymbol(int symbol_index) const {
        const auto &it = cache.find(symbol_index);
        if(it != cache.end()) {
            return it->second;
        } else {
            throw runtime_error("symbol " + to_string(symbol_index) + " NOT Found");
        }
    }

    [[nodiscard]] int getSymbolIndex(const wstring &name) const {
        const auto &it = table.find(name);
        if(it != table.end()) {
            return it->second->index;
        } else {
            throw runtime_error("symbol NOT Found");
        }
    }

    void modifySymbol(int idx, const wstring &name, bool terminator, bool start) {
        auto it = cache.find(idx);
        if(it != cache.end()) {
           auto p_sym = it->second;
           p_sym->name = name;
           p_sym->terminator = terminator;
           p_sym->start = start;
        }
    }

    [[nodiscard]] const Symbol *getStartSymbol() const {
        for(const auto & symbol : getAllSymbols()) {
            if(symbol->start) {
                return symbol;
            }
        }

        throw runtime_error("start symbol NOT Found");
    }



};

// 产生式
struct Production {
    const int left;
    const vector<int> right;

    Production(int left, vector<int> right): left(left), right(std::move(right)) {}

};

// 语法资源池
class GrammarResourcePool {

    // 符号表
    SymbolTable symbolTable;

    // 产生式
    vector<const Production *> productions;

    // FIRST结果存储表
    map<int, const set<int> *> firsts;

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

public:

    const set<int > *FIRST(const vector<int> &symbols, int start_index) {

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

    const set<int>* FIRST(int symbol) {

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
                const Production *p_pdt = production;

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

        this->firsts.insert(pair<int, const set<int> *>(symbol, non_terminator_symbols));

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

    void parse_production_string_line(const wstring &temp_line) {
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

    [[nodiscard]] const vector<const Production *> &get_productions() const {
        return productions;
    }

    [[nodiscard]] const Symbol *getSymbol(int symbol_index) const {
        return symbolTable.getSymbol(symbol_index);
    }

    [[nodiscard]] const Symbol *getStartSymbol() const {
       return symbolTable.getStartSymbol();
    }

    int addSymbol(const wstring &name, bool terminator) {
        return symbolTable.addSymbol(name, terminator);
    }

    const Production *addProduction(int left, initializer_list<int> right) {
        vector<int> right_vector;
        for(int symbol : right) {
            right_vector.push_back(symbol);
        }
        auto p_pdt = new Production(left, right_vector);
        productions.push_back(p_pdt);
        return p_pdt;
    }

    [[nodiscard]] const vector<const Symbol *> &getAllSymbols() const {
        return symbolTable.getAllSymbols();
    }

    void modifySymbol(int index, const wstring &name, bool terminator, bool start) {
        symbolTable.modifySymbol(index, name, terminator, start);
    }
};

// 项
class Item{
    // 对应的产生式
    const Production* const production;

    // 点的位置
    int dot_index = 0;

    const int terminator = 0;

public:

    const bool generated = false;

    explicit Item(const Production *p_pdt, int m_terminator, bool m_generated = false)
        : production(p_pdt), terminator(m_terminator), generated(m_generated) {}

    void set_dot_index(int m_dot_index) {
        if(m_dot_index > production->right.size()) {
            throw runtime_error("DOT_INDEX out of range");
        }
        this->dot_index = m_dot_index;
    }

    [[nodiscard]] int get_dot_index() const {
        return dot_index;
    }

    [[nodiscard]] int get_dot_next_symbol() const {
        if(get_dot_index() == production->right.size()) {
            return 0;
        } else {
            return production->right[dot_index];
        }
    }

    [[nodiscard]] int get_dot_next_i_symbol(int i) const {
        if(get_dot_index() + i >= production->right.size()) {
            return 0;
        } else {
            return production->right[dot_index + i];
        }
    }

    [[nodiscard]] int get_terminator() const {
        return terminator;
    }

    [[nodiscard]] const Production *get_production() const {
        return production;
    }
};

class ItemCollectionManager;

class ItemCollection{

    int index = 0;

    map<size_t, Item *> items;

    vector<Item *> cache;

    GrammarResourcePool *pool;

    friend ItemCollectionManager;

    template <class T>
    inline void hash_combine(std::size_t& seed, const T& v) const
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    }

    static bool compare_item_ptr(const Item* lhs, const Item* rhs)
    {
        if(lhs->get_production() != rhs->get_production())
            return lhs->get_production() < rhs->get_production();
        else if(lhs->get_dot_index() != rhs->get_dot_index())
            return lhs->get_dot_index() < rhs->get_dot_index();
        else
            return lhs->get_terminator() < rhs->get_terminator();
    }

public:

    explicit ItemCollection(GrammarResourcePool *pool) : pool(pool) {

    }

    [[nodiscard]] const vector<Item *> &getItems() const {
        return cache;
    }

    [[nodiscard]] int getIndex() const  {
        return index;
    }


    bool addItem(const Production *p_pdt, int dot_index, int terminator, bool generated = false) {
        auto hasher = hash<int>();
        size_t seed = hasher(reinterpret_cast<const int>(p_pdt));
        hash_combine(seed, dot_index);
        hash_combine(seed, terminator);

        auto it = items.find(seed);
        if(it != items.end()) {
            return false;
        }

        auto *p_item = new Item(p_pdt, terminator, generated);
        p_item->set_dot_index(dot_index);
        items.insert(pair<size_t, Item *>(seed, p_item));
        cache.push_back(p_item);

        return true;
    }

    void CLOSURE() {

        bool ifAdd = true;

        while(ifAdd) {
            ifAdd = false;

            for(const auto & item : items) {
                int next_symbol = item.second->get_dot_next_symbol();

                if(next_symbol == 0
                    || pool->getSymbol(next_symbol)->terminator) {
                    continue;
                }

               for(auto *production : pool->get_productions()) {
                   if(production->left == next_symbol) {
                       vector<int> first_args;
                       first_args.push_back(item.second->get_dot_next_i_symbol(1));
                       first_args.push_back(item.second->get_terminator());

                       const auto first_set = pool->FIRST(first_args, 0);
                       for(auto terminator : *first_set) {
                           if(terminator == 0) continue;
                           if(this->addItem(production, 0, terminator, true)) {
                               ifAdd = true;
                           }
                       }
                   }
               }

            }

        }
    }

    void print() const {

        wcout << L"I" << index << L": ";

        for(const auto item : cache) {
            const auto *p_pdt = item->get_production();
            int dot_index = item->get_dot_index();
            wcout << pool->getSymbol(p_pdt->left)->name << L" -> " ;
            int i = 0;
            for(const auto &symbol : p_pdt->right) {
               if(i++ == dot_index) wcout << L'*';
               wcout << pool->getSymbol(symbol)->name;
            }

            if(i++ == dot_index) wcout << L'*';

            wcout << L',' << pool->getSymbol(item->get_terminator())->name << endl;
        }
        cout << endl;
    }

    [[nodiscard]] size_t getHash() const {
        size_t seed = 0;

        vector<Item *> cache_sorted(cache.begin(), cache.end());
        sort(cache_sorted.begin(), cache_sorted.end(), compare_item_ptr);

        for(const auto item : cache_sorted) {

            if(item->generated) {
                continue;
            }

            hash_combine(seed, item->get_production());
            hash_combine(seed, item->get_dot_index());
            hash_combine(seed, item->get_terminator());
        }
        return seed;
    }
};

class ItemCollectionManager{

    int index = 0;

    map<size_t, ItemCollection *> ic_map;

    map<size_t, ItemCollection *> ic_content_map;

    vector<const ItemCollection *> ics;

    GrammarResourcePool *pool;

    const Production *start_pdt{};

    template <class T>
    inline void hash_combine(std::size_t& seed, const T& v) const
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    }

public:

    explicit ItemCollectionManager(GrammarResourcePool *resource_pool) : pool(resource_pool) {

    }

    void buildItems() {

        const auto startSymbol = pool->getStartSymbol();

        wstring new_symbol_name = startSymbol->name + L"'";

        int new_symbol_index = pool->addSymbol(new_symbol_name, startSymbol->terminator);

        pool->modifySymbol(startSymbol->index, startSymbol->name.substr(1), false, false);

        const auto *p_pdt = pool->addProduction(new_symbol_index, {startSymbol->index});

        this->start_pdt = p_pdt;

        auto *pi_ic = new ItemCollection(pool);

        // -1 代表 $
        pi_ic->addItem(p_pdt, 0, -1);

        pi_ic->CLOSURE();

        addItemCollection(0, 0, pi_ic);

        bool ifAdd = true;

        while(ifAdd) {

            ifAdd = false;
            const auto &r_ics = getItemCollections();
            vector<const ItemCollection *> temp_ics(r_ics.begin(), r_ics.end());
            for(const auto ic : temp_ics) {
                for(const auto symbol : pool->getAllSymbols()) {
                    if(symbol->index <= 0) {
                        continue;
                    }
                    if(GOTO(ic, symbol->index)) {
                        ifAdd = true;
                    }
                }
            }

        }

    }

    [[nodiscard]] const Production *getStartProduction() const {
        return start_pdt;
    }

    [[nodiscard]] const vector<const ItemCollection *> &getItemCollections() const{
        return ics;
    }

    ItemCollection *getItemCollectionByHash(size_t hash) {
        ItemCollection *p_ic = nullptr;
        auto it = ic_content_map.find(hash);
        if(it != ic_content_map.end()) {
            p_ic = it->second;
        }
        return p_ic;
    }

    bool addItemCollection(int idx, int symbol, ItemCollection *p_ic){

        size_t ic_hash = p_ic->getHash();
        auto it = ic_content_map.find(ic_hash);
        if (it != ic_content_map.end()) {
            p_ic = it->second;
        } else {
            p_ic->index = this->index++;
            ic_content_map.insert(pair<size_t, ItemCollection *>(ic_hash, p_ic));
            ics.push_back(p_ic);
        }

        auto hasher = hash<int>();
        size_t seed = hasher(idx);
        hash_combine(seed, symbol);

        auto it2 = ic_map.find(seed);
        if(it2 != ic_map.end()) {
            return false;
        }

        if(symbol != 0)
            wcout << L"GOTO(" << idx << L", " << pool->getSymbol(symbol)->name << L")" << endl;

        ic_map.insert(pair<size_t, ItemCollection *>(seed, p_ic));
        p_ic->print();
        return true;

    }

    [[nodiscard]] const ItemCollection* getGOTO(int idx, int symbol) const {

        auto hasher = hash<int>();
        size_t seed = hasher(idx);
        hash_combine(seed, symbol);

        auto it = ic_map.find(seed);
        if(it != ic_map.end()) {
            return it->second;
        } else {
            return nullptr;
        }
    }

    bool GOTO(const ItemCollection *p_ic, int symbol) {
        auto *pt_ic = new ItemCollection(pool);

        for(const auto &item : p_ic->cache) {
           if(item->get_dot_next_symbol() == symbol) {
               pt_ic->addItem(item->get_production(), item->get_dot_index() + 1, item->get_terminator());
           }
        }
        auto p_temp_ic = this->getItemCollectionByHash(pt_ic->getHash());
        if(p_temp_ic == nullptr)
            pt_ic->CLOSURE();
        else pt_ic = p_temp_ic;

        if(!pt_ic->items.empty()) {
            return this->addItemCollection(p_ic->index, symbol, pt_ic);
        } else {
            return false;
        }

    }

};

class AnalyseTableGenerator {

    using Action = enum {
        MOVE, STATUTE, ACC, STEP_GOTO
    };

    struct Step {

        const Action action;
        union Target{
            int index;
            const Production *production;
        } target{};

        Step(Action action, int index) : action(action), target(Target{index}){}
        Step(Action action, const Production *p_pdt) : action(action) {
            target.production = p_pdt;
        }
    };

    map<size_t, Step *> ACTION;

    map<size_t, Step *> GOTO;

    const ItemCollectionManager *icm;

    const GrammarResourcePool *pool;

    template <class T>
    inline void hash_combine(std::size_t& seed, const T& v) const
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    }

    void add_action(int index, int terminator_symbol, Action action, const Production *target_pdt) {
        size_t seed = 0;
        hash_combine(seed, index);
        hash_combine(seed, terminator_symbol);

        auto it = ACTION.find(seed);
        if(it == ACTION.end()) {
            auto step = new Step(action, target_pdt);
            ACTION.insert(pair<size_t, Step *>(seed, step));
        } else {
            if(it->second->action != action || it->second->target.production != target_pdt)
                throw runtime_error("Conflict Occurred, Syntax NOT LR(1)");
        }
}

void add_action(int index, int terminator_symbol, Action action, int target_index) {
        size_t seed = 0;
        hash_combine(seed, index);
        hash_combine(seed, terminator_symbol);

        auto it = ACTION.find(seed);
        if(it == ACTION.end()) {
            auto step = new Step(action, target_index);
            ACTION.insert(pair<size_t, Step *>(seed, step));
        } else {
            if(it->second->action != action || it->second->target.index != target_index)
                throw runtime_error("Conflict Occurred, Syntax NOT LR(1)");
        }
    }

    void add_goto(int index, int non_terminator_symbol, int target_index) {
        size_t seed = 0;
        hash_combine(seed, index);
        hash_combine(seed, non_terminator_symbol);

        auto it = GOTO.find(seed);
        if(it == GOTO.end()) {
            auto step = new Step(STEP_GOTO, target_index);
            GOTO.insert(pair<size_t, Step *>(seed, step));
        } else {
            if(it->second->target.index != target_index)
                throw runtime_error("Conflict Occurred, Syntax NOT LR(1)");
        }
    }

public:

    explicit AnalyseTableGenerator(const GrammarResourcePool *p_pool, const ItemCollectionManager *p_icm)
    :pool(p_pool) , icm(p_icm) {

    }

    void generate() {
       const auto &ics = icm->getItemCollections();
       for(const auto *ic : ics) {
           for(const auto *item : ic->getItems()) {
               if(item->get_production() == icm->getStartProduction()
                    && item->get_dot_next_symbol() == 0
                    && item->get_terminator() == -1) {
                   this->add_action(ic->getIndex(), -1, ACC, 0);
               }
               int next_symbol = item->get_dot_next_symbol();
                if(next_symbol != 0) {
                    const auto *p_ic = icm->getGOTO(ic->getIndex(), next_symbol);
                    if(pool->getSymbol(next_symbol)->terminator) {
                        if (p_ic != nullptr) {
                            this->add_action(ic->getIndex(), next_symbol, MOVE, p_ic->getIndex());
                        }
                    } else {
                        if (p_ic != nullptr) {
                            this->add_goto(ic->getIndex(), next_symbol, p_ic->getIndex());
                        }
                    }
                } else {
                    if(item->get_production()->left != pool->getStartSymbol()->index) {
                        this->add_action(ic->getIndex(), next_symbol, STATUTE, item->get_production());
                    }
                }
           }
       }
    }

    const Step *findActionStep(int index, int terminator_symbol) {
        size_t seed = 0;
        hash_combine(seed, index);
        hash_combine(seed, terminator_symbol);

        auto it = ACTION.find(seed);
        if(it != ACTION.end()) {
            return it->second;
        } else {
            return nullptr;
        }
    }

    const Step *findGotoStep(int index, int non_terminator_symbol) {
        size_t seed = 0;
        hash_combine(seed, index);
        hash_combine(seed, non_terminator_symbol);

        auto it = GOTO.find(seed);
        if (it != GOTO.end()) {
            return it->second;
        } else {
            return nullptr;
        }
    }

    void print() {
        wcout << L"ACTION" << endl;
        vector<int> symbols;

        wcout << std::left << std::setw(4) << " ";
        for(const auto *symbol : pool->getAllSymbols()) {
            if(symbol->index == 0) continue;
            if(symbol->terminator) {
                wcout << std::left << std::setw(4) << symbol->name;
                symbols.push_back(symbol->index);
            }
        }
        wcout << endl;

        for(int i = 0; i < icm->getItemCollections().size(); i++){
            wcout << std::left << std::setw(4) << i;
            for(int symbol : symbols) {
                auto p_step = this->findActionStep(i, symbol);
                if(p_step == nullptr) {
                    wcout << std::left << std::setw(4) << " ";
                } else {
                    if(p_step->action == MOVE)
                        wcout << std::left << std::setw(4) << wstring(L"s") + to_wstring(p_step->target.index);
                    else if(p_step->action == ACC)
                        wcout << std::left << std::setw(4) << L"acc";
                    else if(p_step->action == STATUTE)
                        wcout << std::left << std::setw(4) << L"r";
                }
            }
            wcout << endl;
        }
    }

};

class Generator{

    // 文件输入
    wifstream input;

    GrammarResourcePool *pool;

    map<string, ItemCollection *> C;

    ItemCollectionManager *icm;

    AnalyseTableGenerator *atg;

public:

    Generator(): input("syntaxInput.txt", std::ios::binary),
        pool(new GrammarResourcePool()),
        icm(new ItemCollectionManager(pool)),
        atg(new AnalyseTableGenerator(pool, icm)){

        auto* codeCvtToUTF8= new std::codecvt_utf8<wchar_t>;

        input.imbue(std::locale(input.getloc(), codeCvtToUTF8));
    }

    void run() {
        pool->FOLLOW();
        icm->buildItems();
        atg->generate();
        atg->print();
    }

    // 得到所有的产生式
    void getProductions() {

        // 读入文法文件
        wstring temp_line;

        while (getline(input, temp_line)) {
            pool->parse_production_string_line(temp_line);
        }
    }

};


int main() {
    clock_t start,end;//定义clock_t变量
    start = clock(); //开始时间


    Generator generator;

    generator.getProductions();

    generator.run();
    //输出时间
    end = clock();   //结束时间
    cout << endl;
    double times = double(end-start)/CLOCKS_PER_SEC;
    cout<<"The Run time = "<<times<<"s" << " = " <<times * 1000 <<"ms" << endl;
    return 0;
}


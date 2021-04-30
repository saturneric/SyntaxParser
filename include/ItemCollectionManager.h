//
// Created by Administrator on 2021/4/30.
//

#ifndef SYNTAXPARSER_ITEMCOLLECTIONMANAGER_H
#define SYNTAXPARSER_ITEMCOLLECTIONMANAGER_H

#include <fstream>
#include <codecvt>

#include <ItemCollection.h>


class ItemCollectionManager{

    std::wofstream output;

    int index = 0;

    std::map<size_t, ItemCollection *> ic_map;

    std::map<size_t, ItemCollection *> ic_content_map;

    std::vector<const ItemCollection *> ics;

    GrammarResourcePool *pool;

    const Production *start_pdt{};

    template <class T>
    inline void hash_combine(std::size_t& seed, const T& v) const
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    }

public:

    explicit ItemCollectionManager(GrammarResourcePool *resource_pool) :
    pool(resource_pool),
    output("LR1Automata.txt", std::ios::binary) {

        auto* codeCvtToUTF8= new std::codecvt_utf8<wchar_t>;
        output.imbue(std::locale(output.getloc(), codeCvtToUTF8));

    }

    ~ItemCollectionManager(){
        output.close();
    }

    void buildItems();

    [[nodiscard]] const Production *getStartProduction() const {
        return start_pdt;
    }

    [[nodiscard]] const std::vector<const ItemCollection *> &getItemCollections() const{
        return ics;
    }

    ItemCollection *getItemCollectionByHash(size_t hash);

    bool addItemCollection(int idx, int symbol, ItemCollection *p_ic);

    [[nodiscard]] const ItemCollection* getGOTO(int idx, int symbol) const;

    bool GOTO(const ItemCollection *p_ic, int symbol);

};


#endif //SYNTAXPARSER_ITEMCOLLECTIONMANAGER_H

//
// Created by Administrator on 2021/4/30.
//

#ifndef SYNTAXPARSER_LR1GENERATOR_H
#define SYNTAXPARSER_LR1GENERATOR_H

#include <codecvt>

#include <GrammarResourcePool.h>
#include <ItemCollectionManager.h>
#include <AnalyseTableGenerator.h>


class LR1Generator{

    // 文件输入
    std::wifstream input;

    GrammarResourcePool *pool;

    ItemCollectionManager *icm;

    AnalyseTableGenerator *atg;

public:

    LR1Generator(): input("syntaxInput.txt", std::ios::binary),
                    pool(new GrammarResourcePool()),
                    icm(new ItemCollectionManager(pool)),
                    atg(new AnalyseTableGenerator(pool, icm)){

        auto* codeCvtToUTF8= new std::codecvt_utf8<wchar_t>;

        input.imbue(std::locale(input.getloc(), codeCvtToUTF8));
    }

    ~LR1Generator() {
        input.close();
    }

    void run() {
        pool->FOLLOW();
        icm->buildItems();
        atg->generate();
        atg->print();
    }

    // 得到所有的产生式
    void getProductions();

    void output(const GrammarResourcePool *&m_pool, const AnalyseTableGenerator *&m_atg) {
        m_pool = this->pool;
        m_atg = this->atg;
    }

};


#endif //SYNTAXPARSER_LR1GENERATOR_H

//
// Created by Administrator on 2021/4/30.
//

#ifndef SYNTAXPARSER_ANALYSETABLEGENERATOR_H
#define SYNTAXPARSER_ANALYSETABLEGENERATOR_H

#include <fstream>
#include <iomanip>

#include <GrammarResourcePool.h>
#include <ItemCollectionManager.h>


using Action = enum {
    MOVE, REDUCE, ACC, STEP_GOTO
};


class AnalyseTableGenerator {

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

    std::map<size_t, Step *> ACTION;

    std::map<size_t, Step *> GOTO;

    const ItemCollectionManager *icm;

    const GrammarResourcePool *pool;

    template <class T>
    inline void hash_combine(std::size_t& seed, const T& v) const
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    }

    void add_action(int index, int terminator_symbol, Action action, const Production *target_pdt);

    void add_action(int index, int terminator_symbol, Action action, int target_index);

    void add_goto(int index, int non_terminator_symbol, int target_index);

public:

    explicit AnalyseTableGenerator(const GrammarResourcePool *p_pool, const ItemCollectionManager *p_icm)
            :pool(p_pool) , icm(p_icm) {}

    void generate();

    const Step *findActionStep(int index, int terminator_symbol) const;

    const Step *findGotoStep(int index, int non_terminator_symbol) const;

    void print() const;

};


#endif //SYNTAXPARSER_ANALYSETABLEGENERATOR_H

//
// Created by Administrator on 2021/4/30.
//

#ifndef SYNTAXPARSER_ITEM_H
#define SYNTAXPARSER_ITEM_H

#include <stdexcept>

#include <Production.h>

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

    void set_dot_index(int m_dot_index);

    [[nodiscard]] int get_dot_index() const {
        return dot_index;
    }

    [[nodiscard]] size_t get_right_size() {
        return production->right.size();
    }

    int get_dot_next_symbol() const;

    int get_dot_next_i_symbol(int i) const;

    [[nodiscard]] int get_terminator() const {
        return terminator;
    }

    [[nodiscard]] const Production *get_production() const {
        return production;
    }
};


#endif //SYNTAXPARSER_ITEM_H

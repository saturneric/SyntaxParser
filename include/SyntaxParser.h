//
// Created by Administrator on 2021/4/30.
//

#ifndef SYNTAXPARSER_SYNTAXPARSER_H
#define SYNTAXPARSER_SYNTAXPARSER_H

#include <queue>
#include <stack>
#include <regex>
#include <codecvt>

#include <GrammarResourcePool.h>
#include <AnalyseTableGenerator.h>
#include <SyntaxTree.h>



class SyntaxParser {

    // 文件输入
    std::wifstream input;

    std::wofstream output;

    std::wofstream treeOutput;

    const GrammarResourcePool *pool;

    const AnalyseTableGenerator *atg;

    std::queue<int> tokens_queue;

    std::stack<int> analyse_stack;

    std::stack<int> status_stack;

    std::stack<TreeNode *> tree_stack;

    std::vector<size_t> lines_index;

    std::wstringstream string_buffer;

    size_t now_line = 1;

    static std::vector<std::wstring> ws_split(const std::wstring& in, const std::wstring& delim);

    static std::pair<std::wstring, std::wstring> get_token_info(const std::wstring &token);

    SyntaxTree syntaxTree;

public:

    SyntaxParser(const GrammarResourcePool *pool, const AnalyseTableGenerator *atg):
            input("tokenOut.txt", std::ios::binary),
            pool(pool),
            atg(atg),
            output("AnalyseOut.txt", std::ios::binary),
            treeOutput("SyntaxOut.txt", std::ios::binary){

        auto* codeCvtToUTF8= new std::codecvt_utf8<wchar_t>;
        input.imbue(std::locale(input.getloc(), codeCvtToUTF8));
        output.imbue(std::locale(output.getloc(), codeCvtToUTF8));
        treeOutput.imbue(std::locale(output.getloc(), codeCvtToUTF8));
    }

    ~SyntaxParser() {
        input.close();
        output.close();
        treeOutput.close();
    }

    // 得到所有的产生式
    void getToken();

    void printSymbol(int symbol_index);

    void printProduction(const Production *p_pdt);

    // 自底向上语法分析
    void parse();

    void printError(std::wofstream &errOutput);

    void printError();

    void printDone();

};


#endif //SYNTAXPARSER_SYNTAXPARSER_H

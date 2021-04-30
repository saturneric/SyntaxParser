#include <iostream>
#include <ctime>

#include <Automata.h>

#include <SymbolTable.h>
#include <GrammarResourcePool.h>
#include <AnalyseTableGenerator.h>

#include <LR1Generator.h>
#include <SyntaxParser.h>

using std::vector;
using std::wstring;

using std::wcout;
using std::endl;


int main(int argc, const char* argv[]) {

    try {

        printf("Compile Program Based on LR(1) Written By Saturneric\n");

        if (argc < 2) {
            printf("Usage: <Input Path>\n");
            return -1;
        }

        clock_t start, end;
        start = clock();

        Automata atm(argv[1]);

        atm.parse();

        atm.output();

        // 输出时间
        end = clock();
        double times = double(end - start) / CLOCKS_PER_SEC;
        wcout << "Token Automata Run time = " << times << "s MicroSeconds" << " = " << times * 1000 << "ms" << endl;

        // LR1生成
        start = clock();

        const GrammarResourcePool *pool;

        const AnalyseTableGenerator *atg;


        LR1Generator generator;

        generator.getProductions();

        generator.run();

        generator.output(pool, atg);

        //输出时间
        end = clock();   //结束时间
        times = double(end - start) / CLOCKS_PER_SEC;
        wcout << "LR(1) Generator Run time = " << times << "s MicroSeconds" << " = " << times * 1000 << "ms" << endl;


        // 语法分析
        start = clock(); //开始时间

        SyntaxParser syntaxParser(pool, atg);

        syntaxParser.getToken();

        syntaxParser.parse();

        //输出时间
        end = clock();   //结束时间
        times = double(end - start) / CLOCKS_PER_SEC;
        wcout << "Syntax Parser Run time = " << times << "s MicroSeconds " << " = " << times * 1000 << "ms" << endl;

    } catch(std::runtime_error &e) {
        std::wcout << "Runtime Error: " << e.what() << endl;
    }

    return 0;
}


//
// Created by Administrator on 2021/4/30.
//

#include "SyntaxParser.h"

void SyntaxParser::parse() {
    status_stack.push(0);

    now_line = 1;
    size_t _line_index = 0, max_line_index = lines_index[now_line-1];
    while(!tokens_queue.empty()) {

        auto *p_step = atg->findActionStep(status_stack.top(), tokens_queue.front());

        if(p_step == nullptr) {
            printError();
            return;
        }

        if(p_step->action == MOVE) {
            output << "MOVE IN" << "(AUTOMATA STATUS " << status_stack.top() <<"): ";
            printSymbol(tokens_queue.front());

            status_stack.push(p_step->target.index);
            analyse_stack.push(tokens_queue.front());

            if(_line_index > max_line_index) {
                string_buffer.str(L"");
                string_buffer.clear();
                max_line_index = lines_index[now_line++];
            }
            string_buffer << pool->getSymbol(tokens_queue.front())->name << " ";
            tokens_queue.pop();
            _line_index++;
        }
        else if(p_step->action == REDUCE) {

            auto *p_pdt = p_step->target.production;
            output << "REDUCE BY" << "(AUTOMATA STATUS " << status_stack.top() <<"): [";
            printProduction(p_pdt);
            output << "]";

            for(int i : p_pdt->right) {
                if(i == 0)
                    continue;
                analyse_stack.pop();
                status_stack.pop();
            }

            auto *p_goto_step =
                    atg->findGotoStep(status_stack.top(), p_pdt->left);

            if(p_goto_step == nullptr) {
                printError();
                return;
            }

            analyse_stack.push(p_pdt->left);
            status_stack.push(p_goto_step->target.index);

        } else if (p_step->action == ACC) {
            output << "ACC";
            printDone();
            return;
        } else {
            printError();
            return;
        }

        output << std::endl;
    }
}

void SyntaxParser::printProduction(const Production *p_pdt) {
    output << pool->getSymbol(p_pdt->left)->name << L" -> " ;
    int i = 0;
    for(const auto &symbol_index : p_pdt->right) {

        if(i++ > 0)  output << " ";

        printSymbol(symbol_index);

    }
}

void SyntaxParser::printSymbol(int symbol_index) {
    auto *symbol = pool->getSymbol(symbol_index);

    if(!symbol->index) {
        output << L"[Epsilon]";
        return;
    }
    if(!symbol->terminator)
        output << pool->getSymbol(symbol_index)->name;
    else
        output << L'"' << pool->getSymbol(symbol_index)->name << L'"';
}

void SyntaxParser::getToken() {

    // 读入文法文件
    std::wstring temp_line;

    size_t _line_index = 0;
    while (getline(input, temp_line)) {
        if(temp_line.size() > 2 && temp_line[0] != '#') {
            std::vector<std::wstring> tokens = ws_split(temp_line, L" ");

            for(int i = 1; i < tokens.size(); i++) {
                if(tokens[i] == L"\r") continue;;
                auto token_info = get_token_info(tokens[i]);
                int symbol_index;

                symbol_index = pool->getSymbolIndex(token_info.first);

                tokens_queue.push(symbol_index);
                _line_index++;

            }
            lines_index.push_back(_line_index - 1);

        }
    }

    // 加入终结符$
    tokens_queue.push(-1);
}

std::vector<std::wstring> SyntaxParser::ws_split(const std::wstring &in, const std::wstring &delim) {
    std::wregex re{ delim };
    return std::vector<std::wstring> {
            std::wsregex_token_iterator(in.begin(), in.end(), re, -1),
            std::wsregex_token_iterator()
    };
}

std::pair<std::wstring, std::wstring> SyntaxParser::get_token_info(const std::wstring &token) {

    auto pre_index = token.find(L'(');

    auto back_index = token.find(L')');

    std::wstring name = token.substr(0, pre_index);
    std::wstring value = token.substr(pre_index + 1, back_index - pre_index - 1);

    return std::pair<std::wstring, std::wstring>(name, value);

}

void SyntaxParser::printDone() {
    output << std::endl;
    output << "------------------------------------------------------" << std::endl;
    output << "Syntax Parser Work Done, No Error Found." << std::endl << std::endl;
}

void SyntaxParser::printError() {
    std::wstring temp_line = string_buffer.str();
    output << std::endl;
    output << "------------------------------------------------------" << std::endl;
    output.fill('-');
    output.width(24);
    output << "Syntax Parser Found Error: " << std::endl
               << "At [Line " << now_line << "]: " << temp_line
               << "<- Next Token{" << pool->getSymbol(tokens_queue.front())->name << "}" << std::endl;
    output << "AUTOMATA STATUS " << status_stack.top() << std::endl;
}

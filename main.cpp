#include <iostream>
#include <ctime>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <set>
#include <map>

using namespace std;

using std::vector;
using std::string;
using std::stringstream;
using std::pair;

struct Symbol {

    const int index;
    const string name;
    const bool terminator;

    Symbol(int index, const string & name, bool terminator):
    index(index),
    name(name),
    terminator(terminator)
    {}

};

class SymbolTable {
    int index = 0;
    map<string, Symbol> table;

public:

    int addSymbol(const string& name, bool terminator) {
        const auto &it = table.find(name);
        if ((it) != table.end()) {
            return it->second.index;
        }
        Symbol symbol = Symbol(index++, name, terminator);
        table.insert(pair<string, Symbol>(symbol.name, symbol));
        return symbol.index;
    }
};

ifstream input("syntaxInput.txt");


map<string, set<string>> productions;//读入文件后，存放产生式
map<string, set<string>> project;//在每个产生式右部前面加上点，得到初始项目集
map<int, set<string>> StateSet;//状态
map<int, set<string>> StateSetTemp;
string css_by_order[20];
string link[12][12];//存放各个状态之间的关系

SymbolTable symbolTable;

vector<vector<string>> ActionTable;

vector<vector<int>> GoToTable;

set<string> Vn;
set<string> Vt;

vector<string> vn_index;
vector<string> vt_index;

bool isVn(char ch) {
    if (ch >= 'A' && ch <= 'Z') {
        return true;
    } else{
        return false;
    }
}


// 去掉首尾空格
string& trim(string &&str) {
    if (str.empty()) {
        return str;
    }

    str.erase(0,str.find_first_not_of(' '));
    str.erase(str.find_last_not_of(' ') + 1);
    return str;
}

//得到最初项目集
void getProject() {
    productions["S\'"].insert("S");
    //读入文法文件
    int k = 1;
    string temp_line;
    while (getline(input, temp_line)) {
        auto middle_index = temp_line.find("->", 0);


        if(middle_index == string::npos) {
            throw runtime_error("-> NOT FOUND");
        }

        string front = trim(temp_line.substr(0, middle_index));

        string back = trim(temp_line.substr(middle_index + 2, temp_line.size() - middle_index - 2));

        stringstream terminator, non_terminator;
        bool is_terminator = false;
        for(const auto &c : back) {
            if (c == '\"') {
                if(is_terminator) {
                    symbolTable.addSymbol(trim(terminator.str()), true);
                    terminator.str("");
                    terminator.clear();
                }
                is_terminator = !is_terminator;
                continue;
            }
            if(c == ' ') {
                string temp_symbol = trim(non_terminator.str());
                if(!temp_symbol.empty()) {
                    symbolTable.addSymbol(trim(non_terminator.str()), false);
                    non_terminator.str("");
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

        productions[temp_line.substr(0, 1)].insert(temp_line.substr(3, temp_line.length() - 3 + 1));
        css_by_order[k++] = temp_line;
    }
    //构造基本项目集，即圆点位于右部开头的位置
    string dot = "*";
    for(map<string, set<string>>::iterator it = productions.begin(); it != productions.end(); it++) {
        for(set<string>::iterator ij =it->second.begin(); ij != it->second.end(); ij++) {
            string startStr = *ij;
            startStr.insert(0, dot, 0, 1);
            project[it->first].insert(startStr);
        }
    }
}

void Init_I0() {
    StateSet[0].insert("S'->*S");
    //先将所有的左部为S的产生式加入0号状态中
    for(map<string, set<string> >::iterator it = project.begin(); it != project.end(); it++) {
        if ((it->first) == "S") {
            for(set<string>::iterator ij = it->second.begin(); ij != it->second.end(); ij++) {
                StateSetTemp[0].insert(it->first + "->" + *ij);
            }
        }
    }
    //根据I0状态中已知内容，添加后续
    while (! StateSetTemp[0].empty()){
        set<string>::iterator it = StateSetTemp[0].begin();
        string it_str = *it;//暂时集合中的每一条产生式
        //如果点后面是非终结符，在project例找到它能推出来的产生式，加入temp,
        if (isVn(it_str[4])) {
            string after_dot_ch = it_str.substr(4,1);
            for(set<string>::iterator css = project[after_dot_ch].begin(); css != project[after_dot_ch].end(); css++) {
                StateSetTemp[0].insert(after_dot_ch + "->" + *css);
            }
            StateSetTemp[0].erase(it_str);
            StateSet[0].insert(it_str);
        }else if(! isVn(it_str[4])) {
            StateSetTemp[0].erase(it_str);
            StateSet[0].insert(it_str);
        }
    }
}

string MoveDot(string s, int pos) {
    s.erase(pos,1);
    s = s.substr(0, pos+1) + "*" + s.substr(pos+1, s.length()-pos);
    return s;
}

void GenerateState() {
    Init_I0();
    int i = 0;
    int state_num = 0;
    string enter;
    //对于每一个状态
    while (i < StateSet.size()) {
        map<string, set<string>> after_dot_ch;
        //构造该状态中的after_dot_ch的map，first为点后面的字母，即输入字母，second为输入字母为first时的项目
        for (set<string>::iterator it = StateSet[i].begin(); it != StateSet[i].end(); it++) {//对于此状态的每一个项目
            int dot_pos = (*it).find("*");
            if (dot_pos + 1 != (*it).length()) {
                string s = (*it).substr(dot_pos + 1, 1);//点后面的字母
                after_dot_ch[s].insert(*it);
            }
        }
        if (after_dot_ch.size() == 0) {
            i++;
            continue;
        }
        for (map<string, set<string>>::iterator adc = after_dot_ch.begin(); adc != after_dot_ch.end(); adc++) {
            enter = adc->first;
            state_num++;
            for (set<string>::iterator p = adc->second.begin(); p != adc->second.end(); p++) {
                int dot_pos = (*p).find("*");
                string moved_proj = MoveDot(*p, dot_pos);
                int moved_dot_pos = (moved_proj).find("*");
                StateSet[state_num].insert(moved_proj);

                string moved_adc = moved_proj.substr(moved_dot_pos + 1, 1);
                if (isVn(moved_adc[0])) {
                    //如果点移动后的字符串中，点后字母是非终结符，则将以该非终结符开头的project加入该状态
                    for (set<string>::iterator proj_css = project[moved_adc].begin();
                         proj_css != project[moved_adc].end(); proj_css++) {
                        StateSet[state_num].insert(moved_adc + "->" + (*proj_css));
                    }
                }
            }
            //--------判断之前是否有与新产生的状态相等的--------
            bool is_equal = false;
            for (int j = 0; j < state_num; ++j) {
                if (StateSet[state_num].size() == StateSet[j].size()) {
                    for (set<string>::iterator str = StateSet[j].begin(); str != StateSet[j].end(); str++) {
                        if (StateSet[state_num].count(*str) == 0){
                            break;
                        } else{
                            link[i][j] = enter;
                            is_equal = true;
                        }
                    }
                }
            }
            if (is_equal == true) {//两个状态全部项目都相等
                StateSet.erase(state_num);
                state_num--;
            } else{
                link[i][state_num] = enter;
            }
            //--------------------------------------------
        }
        i++;
    }
}

void printState() {
    cout << "-------------------------Status--------------------------"<<endl;
    for(map<int, set<string>>::iterator it = StateSet.begin(); it != StateSet.end(); it++) {
        cout << "I_" << it->first << ": ";
        for(set<string>::iterator ij = it->second.begin(); ij != it->second.end(); ij++) {
            cout << *ij << ", ";
        }
        cout << endl;
    }
    cout << "---------------------------------------------------------"<<endl;
}

void printLinkedTable() {
    cout << "============================================DFA================================================"<<endl;
    cout <<  '\t';
    for (int i = 0; i < 11; ++i) {
        cout << i << '\t';
    }
    cout << endl;
    for (int i = 0; i < 11; ++i) {
        cout << i << '\t';
        for (int j = 0; j < 11; ++j) {
            cout << link[i][j] << '\t';
        }
        cout << endl;
    }
}

void getVnandVt() {
    for(map<string, set<string>>::iterator it = productions.begin(); it != productions.end(); it++) {
        string c = it->first;
        if (c != "S'")
            Vn.insert(c);
        for(set<string>::iterator ij = it->second.begin(); ij != it->second.end(); ij++) {
            for (int i = 0; i < (*ij).length(); ++i) {
                if (!isVn((*ij)[i])) {
                    Vt.insert((*ij).substr(i, 1));
                }
            }
        }
    }
    Vt.insert("#");
}

int findIndex(string enter) {
    int pos;
    if (!isVn(enter[0])) {
        for (int k = 0; k < 10; ++k) {
            if (vt_index[k] == enter) {
                pos = k;
                break;
            }
        }
    } else{
        for (int k = 0; k < 10; ++k) {
            if (vn_index[k] == enter) {
                pos = k;
                break;
            }
        }
    }
    return pos;
}

void GenerateAction_Table() {
    //构造Action分析表的行名与列名
    ActionTable[0][0] = "";
    int j = 0;
    set<string>::iterator ij = Vt.begin();
    while (j < Vt.size()) {
        vt_index[j] = *ij;
        ij++; j++;
    }

    //构造Action表
    // ---------移入
    for (int i = 0; i < StateSet.size(); ++i) {
        for (int j = 0; j < StateSet.size(); ++j) {
            if (link[i][j].length() != 0 && (!isVn(link[i][j][0]))) {
                string enter = link[i][j];
                int pos = findIndex(enter);
                ActionTable[i][pos] = "s" + to_string(j);
            }
        }
    }
    // ---------规约
    for (int l = 0; l < StateSet.size(); ++l) {
        if (StateSet[l].size() == 1) {
            set<string>::iterator it = StateSet[l].begin();
            string proj = *it;
            if (proj == "S'->S*") {
                ActionTable[l][findIndex("#")] = "acc";
                continue;
            }
            int dot_pos = proj.find("*");
            if (dot_pos+1 == proj.length()) {
                string no_dot_proj = proj.erase(dot_pos,1);
                //先找到这个项目对应的无点产生式index
                int index = 0;//无点产生式的编号index。第几条产生式
                for (index = 0; index < 20; ++index) {
                    if (css_by_order[index] == no_dot_proj) {
                        break;
                    }
                }
                for (int m = 0; m < Vt.size(); ++m) {
                    ActionTable[l][m] = "r" + to_string(index);
                }
            }
        }
    }
    cout << endl;
    cout << endl;
    cout << "----------------------[Action]---------------------" << endl;
    int m = 0;
    cout << '\t';
    while (vt_index[m].length() != 0) {
        cout << vt_index[m] << '\t';
        m++;
    }
    cout << endl;
    for (int k = 0; k < StateSet.size(); ++k) {
        cout << k << '\t';
        for (int l = 0; l < Vt.size(); ++l) {
            cout << ActionTable[k][l] << '\t';
        }
        cout <<endl;
    }
}

void GenerateGoTo_Table() {

    for(const auto& item : Vn) {
        vn_index.push_back(item);
    }

    // 初始化GOTO表
    for(int i = 0; i < StateSet.size(); i++) {
        GoToTable.emplace_back();
        for(int j = 0; i < StateSet.size(); j++){
            GoToTable[i].emplace_back();
        }
    }

    //构造GoTo表
    //在link表中遍历
    for (int i = 0; i < StateSet.size(); ++i) {
        for (int j = 0; j < StateSet.size(); ++j) {
            if ( link[i][j].length() != 0 && isVn(link[i][j][0]) ) {
                string enter = link[i][j];
                int pos = findIndex(enter);
                GoToTable[i][pos] = j;
            }
        }
    }
    cout << endl;
    cout << endl;
    cout << "----------------------[GoTo]---------------------" << endl;
    int m = 0;
    cout << '\t';
    while (vn_index[m].length() != 0) {
        cout << vn_index[m] << '\t';
        m++;
    }
    cout << endl;
    for (int k = 0; k < StateSet.size(); ++k) {
        cout << k << '\t';
        for (int l = 0; l < Vn.size(); ++l) {
            if (GoToTable[k][l] != 0)
                cout << GoToTable[k][l] << '\t';
            else
                cout << '\t';
        }
        cout << endl;
    }
}

int main() {
    clock_t start,end;//定义clock_t变量
    start = clock(); //开始时间

    getProject();
//    GenerateState();
//    printState();
//    printLinkedTable();
//    getVnandVt();
//    GenerateAction_Table();
//    GenerateGoTo_Table();

    //输出时间
    end = clock();   //结束时间
    cout << endl;
    double times = double(end-start)/CLOCKS_PER_SEC;
    cout<<"The Run time = "<<times<<"s" << " = " <<times * 1000 <<"ms" << endl;
    return 0;
}


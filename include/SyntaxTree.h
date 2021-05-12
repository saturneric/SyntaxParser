//
// Created by Administrator on 2021/5/12.
//

#ifndef SYNTAXPARSER_SYNTAXTREE_H
#define SYNTAXPARSER_SYNTAXTREE_H

#include <stack>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>

class TreeNode {
    const int nodeType;
    TreeNode *father = nullptr;
    std::vector<std::wstring> infoVec;
    std::vector<TreeNode *> children;

public:

    explicit TreeNode(int nodeType) : nodeType(nodeType) {

    }

    void addInfo(const std::wstring& info) {
        infoVec.push_back(info);
    }

    void setFather(TreeNode *fatherNode) {
        if(fatherNode == this) {
            throw std::runtime_error("Illegal Tree Structure");
        }
        if(std::find(fatherNode->children.begin(), fatherNode->children.end(), this) == fatherNode->children.end()) {
            fatherNode->children.push_back(this);
        }
        this->father = fatherNode;
    }

    const std::vector<TreeNode *> &getChildren() {
        return children;
    }

    const std::vector<std::wstring> &getInfoVec() {
        return infoVec;
    }
};

class SyntaxTree {

    TreeNode *root = nullptr;

    void do_tree_node_print(TreeNode *thisNode, std::wofstream &stream);
public:

    std::stack<int> tabStack;

    const int spacesInTab = 4;

    void setRoot(TreeNode *node) {
        this->root = node;
    }

    void print(std::wofstream &stream);

};


#endif //SYNTAXPARSER_SYNTAXTREE_H

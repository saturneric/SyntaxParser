//
// Created by Administrator on 2021/5/12.
//

#include "SyntaxTree.h"

void SyntaxTree::do_tree_node_print(TreeNode *thisNode, std::wofstream &stream) {

    tabStack.push(tabStack.top() + 1);

    for(int i = tabStack.top() * spacesInTab; i > 0; i--) {
        if(i % spacesInTab == 0) {
            stream << L'|';
        }
        stream << L' ';
    }


    int count = 0;
    for(auto &info : thisNode->getInfoVec()) {
        if(count++ != 0) {
            if(count != 2) {
                stream << L", ";
            }
            stream << info;
        } else {
            stream << info << L'<';
        }
    }

    stream << L'>';

    stream << L'\n';

    if(!thisNode->getChildren().empty()) {
        for (auto &node : thisNode->getChildren()) {
            do_tree_node_print(node, stream);
        }
    }

    tabStack.pop();
}

void SyntaxTree::print(std::wofstream &stream) {

    if(this->root == nullptr) return;

    tabStack.push(-1);

    do_tree_node_print(root, stream);

}

#include "AST/read.hpp"

// TODO
ReadNode::ReadNode(const uint32_t line, const uint32_t col, AstNode *p_var_ref)
    : AstNode{line, col}, target(p_var_ref) {}

// TODO: You may use code snippets in AstDumper.cpp
void ReadNode::print() {
    AstNode::outputIndentationSpace(m_indentation);
    std::printf("read statement <line: %u, col: %u>\n",
                location.line,
                location.col);    
    AstNode::incrementIndentation();
    if( target!= NULL){
        target->print();
    }

    AstNode::decrementIndentation();
}

// void ReadNode::visitChildNodes(AstNodeVisitor &p_visitor) {
//     // TODO
// }
void ReadNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void ReadNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if( target!= NULL)
        target->accept(p_visitor);
}


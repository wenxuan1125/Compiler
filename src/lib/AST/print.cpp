#include "AST/print.hpp"

// TODO
PrintNode::PrintNode(const uint32_t line, const uint32_t col, AstNode *p_expr)
    : AstNode{line, col}, target(p_expr) {}

// TODO: You may use code snippets in AstDumper.cpp
void PrintNode::print() {
    AstNode::outputIndentationSpace(m_indentation);
    std::printf("print statement <line: %u, col: %u>\n",
                location.line,
                location.col);    
    AstNode::incrementIndentation();
    if( target!= NULL){
        target->print();
    }

    AstNode::decrementIndentation();
}

// void PrintNode::visitChildNodes(AstNodeVisitor &p_visitor) {
//     // TODO
// }

void PrintNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void PrintNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if( target!= NULL)
        target->accept(p_visitor);
}


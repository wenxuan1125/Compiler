#include "AST/return.hpp"

// TODO
ReturnNode::ReturnNode(const uint32_t line, const uint32_t col, AstNode *p_return_var)
    : AstNode{line, col}, return_value(p_return_var) {}

// TODO: You may use code snippets in AstDumper.cpp
void ReturnNode::print() {
    AstNode::outputIndentationSpace(m_indentation);
    std::printf("return statement <line: %u, col: %u>\n",
                location.line,
                location.col);    
    AstNode::incrementIndentation();
    if( return_value!= NULL){
        return_value->print();
    }
    AstNode::decrementIndentation();
}

// void ReturnNode::visitChildNodes(AstNodeVisitor &p_visitor) {
//     // TODO
// }
void ReturnNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void ReturnNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if( return_value!= NULL)
        return_value->accept(p_visitor);
}

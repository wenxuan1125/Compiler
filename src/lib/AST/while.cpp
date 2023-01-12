#include "AST/while.hpp"

// TODO
WhileNode::WhileNode(const uint32_t line, const uint32_t col, AstNode *p_cond, AstNode *p_body)
    : AstNode{line, col}, condition(p_cond), body(p_body) {}

// TODO: You may use code snippets in AstDumper.cpp
void WhileNode::print() {
    AstNode::outputIndentationSpace(m_indentation);
    std::printf("while statement <line: %u, col: %u>\n",
                location.line,
                location.col);    
    AstNode::incrementIndentation();
    if( condition!= NULL){
        condition->print();
    }
    if( body!= NULL){
        body->print();
    }
    AstNode::decrementIndentation();
}

// void WhileNode::visitChildNodes(AstNodeVisitor &p_visitor) {
//     // TODO
// }
void WhileNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void WhileNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if( condition!= NULL)
        condition->accept(p_visitor);
    if( body!= NULL)
        body->accept(p_visitor);
}

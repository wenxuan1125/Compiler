#include "AST/if.hpp"

// TODO
IfNode::IfNode(const uint32_t line, const uint32_t col, AstNode *p_cond, AstNode *p_body, AstNode *p_body_el)
    : AstNode{line, col}, condition(p_cond), body(p_body), body_else(p_body_el) {}

// TODO: You may use code snippets in AstDumper.cpp
void IfNode::print() {
    AstNode::outputIndentationSpace(m_indentation);
    std::printf("if statement <line: %u, col: %u>\n",
                location.line,
                location.col);    
    AstNode::incrementIndentation();
    if( condition!= NULL){
        condition->print();
    }
    if( body!= NULL){
        body->print();
    }
    if( body_else!= NULL){
        body_else->print();
    }
    AstNode::decrementIndentation();
}

// void IfNode::visitChildNodes(AstNodeVisitor &p_visitor) {
//     // TODO
// }
void IfNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void IfNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if( condition!= NULL)
        condition->accept(p_visitor);
    if( body!= NULL)
        body->accept(p_visitor);
    if( body_else!= NULL)
        body_else->accept(p_visitor); 
}

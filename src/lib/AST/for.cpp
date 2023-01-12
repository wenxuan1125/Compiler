#include "AST/for.hpp"

// TODO
ForNode::ForNode(const uint32_t line, const uint32_t col, 
                AstNode *p_loop_var, AstNode *p_init, AstNode *p_cond, AstNode *p_body)
    : AstNode{line, col}, loop_variable(p_loop_var),
     initial(p_init), condition(p_cond), body(p_body) {}

// TODO: You may use code snippets in AstDumper.cpp
void ForNode::print() {
    AstNode::outputIndentationSpace(m_indentation);
    std::printf("for statement <line: %u, col: %u>\n",
                location.line,
                location.col);    
    AstNode::incrementIndentation();
    if( loop_variable!= NULL){
        loop_variable->print();
    }
    if( initial!= NULL){
        initial->print();
    }
    if( condition!= NULL){
        condition->print();
    }
    if( body!= NULL){
        body->print();
    }
    AstNode::decrementIndentation();
}

// void ForNode::visitChildNodes(AstNodeVisitor &p_visitor) {
//     // TODO
// }
void ForNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void ForNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if (loop_variable != NULL)
    {
        loop_variable->accept(p_visitor);
    }
    if (initial != NULL)
    {
        initial->accept(p_visitor);
    }
    if (condition != NULL)
    {
        condition->accept(p_visitor);
    }
    if (body != NULL)
    {
        body->accept(p_visitor);
    }
}

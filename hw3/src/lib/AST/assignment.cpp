#include "AST/assignment.hpp"

// TODO
AssignmentNode::AssignmentNode(const uint32_t line, const uint32_t col, AstNode *p_lval, AstNode *p_expr)
    : AstNode{line, col}, lvalue(p_lval), expression(p_expr) {}

// TODO: You may use code snippets in AstDumper.cpp
void AssignmentNode::print() {
    AstNode::outputIndentationSpace(m_indentation);
    std::printf("assignment statement <line: %u, col: %u>\n",
                location.line, location.col);
                
    AstNode::incrementIndentation();
    if( lvalue!= NULL)
        lvalue->print();
    if( expression!= NULL)
        expression->print();
    AstNode::decrementIndentation();
}

// void AssignmentNode::visitChildNodes(AstNodeVisitor &p_visitor) {
//     // TODO
// }

void AssignmentNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void AssignmentNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if (lvalue != NULL)
    {
        lvalue->accept(p_visitor);
    }
    if (expression != NULL)
    {
        expression->accept(p_visitor);
    }
}


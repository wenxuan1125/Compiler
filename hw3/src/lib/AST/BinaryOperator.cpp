#include "AST/BinaryOperator.hpp"

// TODO
BinaryOperatorNode::BinaryOperatorNode(const uint32_t line, const uint32_t col, Binary_Op p_op,
                                       AstNode *p_left, AstNode *p_right)
    : ExpressionNode{line, col}, op(p_op), left(p_left), right(p_right) {}

// TODO: You may use code snippets in AstDumper.cpp
const char *BinaryOperatorNode::getOperatorCString() const { 
    if( op.mul)
        return "*";
    else if (op.div)
        return "/";
    else if(op.mod)
        return "mod";
    else if(op.add)
        return "+";
    else if(op.sub)
        return "-";
    else if(op.lt)
        return "<";
    else if(op.le)
        return "<=";
    else if(op.eq)
        return "=";
    else if(op.ne)
        return "<>";
    else if(op.ge)
        return ">=";
    else if(op.gt)
        return ">";
    else if(op.AND)
        return "and";
    else if(op.OR)
        return "or";
}
void BinaryOperatorNode::print() {
    AstNode::outputIndentationSpace(m_indentation);
    std::printf("binary operator <line: %u, col: %u> ",
                location.line, location.col);
    
    if( op.mul)
        printf("*\n");
    else if(op.div)
        printf("/\n");
    else if(op.mod)
        printf("mod\n");
    else if(op.add)
        printf("+\n");
    else if(op.sub)
        printf("-\n");
    else if(op.lt)
        printf("<\n");
    else if(op.le)
        printf("<=\n");
    else if(op.eq)
        printf("=\n");
    else if(op.ne)
        printf("<>\n");
    else if(op.ge)
        printf(">=\n");
    else if(op.gt)
        printf(">\n");
    else if(op.AND)
        printf("and\n");
    else if(op.OR)
        printf("or\n");
                
    AstNode::incrementIndentation();
    if (left != NULL)
    {
        left->print();
    }
    if (right != NULL)
    {
        right->print();
    }
    AstNode::decrementIndentation();
}

// void BinaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
//     // TODO
// }
void BinaryOperatorNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void BinaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if( left!= NULL)
        left->accept(p_visitor);
    if( right!= NULL)
        right->accept(p_visitor);
}


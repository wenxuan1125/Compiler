#include "AST/UnaryOperator.hpp"

// TODO
UnaryOperatorNode::UnaryOperatorNode(const uint32_t line, const uint32_t col, Unary_Op p_op,
                      AstNode *p_operand)
    : ExpressionNode{line, col}, op(p_op), operand(p_operand){}

// TODO: You may use code snippets in AstDumper.cpp
const char *UnaryOperatorNode::getOperatorCString() const { 
    if( op.neg)
        return "neg";
    else if (op.NOT)
        return "not";
}
void UnaryOperatorNode::print() {
        AstNode::outputIndentationSpace(m_indentation);
    std::printf("unary operator <line: %u, col: %u> ",
                location.line, location.col);
    
    if( op.neg)
        printf("neg\n");
    else if(op.NOT)
        printf("not\n");
    AstNode::incrementIndentation();
    if (operand != NULL)
    {
        operand->print();
    }
    AstNode::decrementIndentation();
}

// void UnaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
//     // TODO
// }
void UnaryOperatorNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void UnaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if( operand!= NULL)
        operand->accept(p_visitor);
}


#include "AST/print.hpp"
#include "visitor/AstNodeVisitor.hpp"

PrintNode::PrintNode(const uint32_t line, const uint32_t col,
                     ExpressionNode *p_expr)
    : AstNode{line, col}, target(p_expr) {}

void PrintNode::accept(AstNodeVisitor &p_visitor) { p_visitor.visit(*this); }

void PrintNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    target->accept(p_visitor);
}

const int PrintNode::checkInvalidChildren() const{

    if(std::strcmp(target->getPTypeCString(), "null")==0)
        return 1;

    return 0;
}

const uint32_t PrintNode::getTargetLocationCol() const{
    return target->getLocation().col;
} 

const int PrintNode::checkTargetScalarType() const{
    if(std::strcmp(target->getPTypeCString(), "integer")==0 || std::strcmp(target->getPTypeCString(), "real")==0
    || std::strcmp(target->getPTypeCString(), "string")==0 || std::strcmp(target->getPTypeCString(), "boolean")==0)
        return 1;
    else
        return 0;
}


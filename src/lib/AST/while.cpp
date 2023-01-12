#include "AST/while.hpp"
#include "visitor/AstNodeVisitor.hpp"

WhileNode::WhileNode(const uint32_t line, const uint32_t col,
                     ExpressionNode *p_condition, CompoundStatementNode *p_body)
    : AstNode{line, col}, condition(p_condition), body(p_body) {}

void WhileNode::accept(AstNodeVisitor &p_visitor) { p_visitor.visit(*this); }

void WhileNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    condition->accept(p_visitor);
    body->accept(p_visitor);
}

const int WhileNode::checkInvalidCondition() const{

    if(std::strcmp(condition->getPTypeCString(), "null")==0)
        return 1;

    return 0;
}
const int WhileNode::checkConditionBoolType() const{
    if(std::strcmp(condition->getPTypeCString(), "boolean")==0)
        return 1;
    else
        return 0;
}
const uint32_t WhileNode::getConditionLocationCol() const{
    return condition->getLocation().col;
}


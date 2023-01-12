#include "AST/if.hpp"
#include "visitor/AstNodeVisitor.hpp"

IfNode::IfNode(const uint32_t line, const uint32_t col,
               ExpressionNode *p_condition, CompoundStatementNode *p_body,
               CompoundStatementNode *p_else)
    : AstNode{line, col}, condition(p_condition), body(p_body),
      else_body(p_else) {}

void IfNode::accept(AstNodeVisitor &p_visitor) { p_visitor.visit(*this); }

void IfNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    condition->accept(p_visitor);
    body->accept(p_visitor);
    if (else_body) {
        else_body->accept(p_visitor);
    }
}

const int IfNode::checkInvalidCondition() const{

    if(std::strcmp(condition->getPTypeCString(), "null")==0)
        return 1;

    return 0;
}
const int IfNode::checkConditionBoolType() const{
    if(std::strcmp(condition->getPTypeCString(), "boolean")==0)
        return 1;
    else
        return 0;
}
const uint32_t IfNode::getConditionLocationCol() const{
    return condition->getLocation().col;
}


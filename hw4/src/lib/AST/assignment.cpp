#include "AST/assignment.hpp"
#include "visitor/AstNodeVisitor.hpp"

AssignmentNode::AssignmentNode(const uint32_t line, const uint32_t col,
                               VariableReferenceNode *p_lvalue,
                               ExpressionNode *p_expr)
    : AstNode{line, col}, lvalue(p_lvalue), expr(p_expr) {}

void AssignmentNode::accept(AstNodeVisitor &p_visitor)
{
    p_visitor.visit(*this);
}

void AssignmentNode::visitChildNodes(AstNodeVisitor &p_visitor)
{
    lvalue->accept(p_visitor);
    expr->accept(p_visitor);
}

const int AssignmentNode::checkInvalidLvalue() const
{

    if (std::strcmp(lvalue->getPTypeCString(), "null") == 0)
        return 1;

    return 0;
}

const int AssignmentNode::checkInvalidRvalue() const
{

    if (std::strcmp(expr->getPTypeCString(), "null") == 0)
        return 1;

    return 0;
}

const char *AssignmentNode::getLvalueNameCString() const
{
    return lvalue->getNameCString();
}

const int AssignmentNode::checkLvalueScalarType() const
{
    if (std::strcmp(lvalue->getPTypeCString(), "integer") == 0 || std::strcmp(lvalue->getPTypeCString(), "real") == 0 || std::strcmp(lvalue->getPTypeCString(), "string") == 0 || std::strcmp(lvalue->getPTypeCString(), "boolean") == 0)
        return 1;
    else
        return 0;
}
const int AssignmentNode::checkRvalueScalarType() const
{
    if (std::strcmp(expr->getPTypeCString(), "integer") == 0 || std::strcmp(expr->getPTypeCString(), "real") == 0 || std::strcmp(expr->getPTypeCString(), "string") == 0 || std::strcmp(expr->getPTypeCString(), "boolean") == 0)
        return 1;
    else
        return 0;
}
const int AssignmentNode::checkCompatibleLvalueRvalueType() const
{
    if (std::strcmp(expr->getPTypeCString(), lvalue->getPTypeCString()) == 0)
        return 1;
    else if (std::strcmp(lvalue->getPTypeCString(), "real") == 0 && std::strcmp(expr->getPTypeCString(), "integer") == 0)
        return 1;
    else
        return 0;
}
const char *AssignmentNode::getRvalueTypeCString() const
{
    return expr->getPTypeCString();
}
const char *AssignmentNode::getLvalueTypeCString() const
{
    return lvalue->getPTypeCString();
}

const uint32_t AssignmentNode::getLvalueLocationCol() const
{
    return lvalue->getLocation().col;
}
const uint32_t AssignmentNode::getRvalueLocationCol() const
{
    return expr->getLocation().col;
}
const char *AssignmentNode::getConstantValueCString()
{

    //std::shared_ptr<A> foo;
    //std::shared_ptr<ConstantValueNode> expr_constant;
    //expr_constant = std::dynamic_pointer_cast<ConstantValueNode *>(expr);

    std::unique_ptr<ConstantValueNode> expr_constant(dynamic_cast<ConstantValueNode *>(expr.get()));
    if (expr_constant)
        expr.release();

    return expr_constant->getConstantValueCString();
}


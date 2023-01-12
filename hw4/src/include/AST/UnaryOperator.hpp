#ifndef __AST_UNARY_OPERATOR_NODE_H
#define __AST_UNARY_OPERATOR_NODE_H

#include "AST/expression.hpp"
#include "AST/operator.hpp"

#include <memory>
#include <cstring>

class UnaryOperatorNode : public ExpressionNode {
  public:
    UnaryOperatorNode(const uint32_t line, const uint32_t col, Operator op,
                      ExpressionNode *p_operand);
    ~UnaryOperatorNode() = default;

    const char *getOpCString() const;
    const int checkInvalidChildren() const;
    const char *getOperandTypeCString() const;
    void accept(AstNodeVisitor &p_visitor) override;
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

  private:
    Operator op;
    std::unique_ptr<ExpressionNode> operand;
};

#endif


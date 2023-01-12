#ifndef __AST_CONSTANT_VALUE_NODE_H
#define __AST_CONSTANT_VALUE_NODE_H

#include "AST/expression.hpp"

class ConstantValueNode : public ExpressionNode {
  public:
    ConstantValueNode(const uint32_t line, const uint32_t col,
                      /* TODO: constant value */
                      Constant_Value p_constant_value);
    ~ConstantValueNode() = default;

    void visitChildNodes(AstNodeVisitor &p_visitor);
    void accept(AstNodeVisitor &p_visitor) override;
    const int getConstantType() const;
    const char *getStringValue() const;
    const int getIntValue() const;
    const float getRealValue() const;

    void print() override;

  private:
    // TODO: constant value
    Constant_Value constant_value;
};

#endif


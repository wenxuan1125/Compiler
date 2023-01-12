#ifndef __AST_BINARY_OPERATOR_NODE_H
#define __AST_BINARY_OPERATOR_NODE_H

#include "AST/expression.hpp"

#include <memory>

class BinaryOperatorNode : public ExpressionNode {
  public:
    BinaryOperatorNode(const uint32_t line, const uint32_t col, Binary_Op p_op,
                      AstNode *p_left, AstNode *p_right
                       /* TODO: operator, expressions */);
    ~BinaryOperatorNode() = default;

    void visitChildNodes(AstNodeVisitor &p_visitor);
    void accept(AstNodeVisitor &p_visitor) override;
    const char *getOperatorCString() const;

    void print() override;

  private:
    // TODO: operator, expressions
    
    AstNode *left;
    AstNode *right;
    Binary_Op op;
};

#endif


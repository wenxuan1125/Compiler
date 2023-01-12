#ifndef __AST_WHILE_NODE_H
#define __AST_WHILE_NODE_H

#include "AST/ast.hpp"

class WhileNode : public AstNode {
  public:
    WhileNode(const uint32_t line, const uint32_t col, AstNode *p_cond, AstNode *p_body
              /* TODO: expression, compound statement */);
    ~WhileNode() = default;

    void visitChildNodes(AstNodeVisitor &p_visitor);
    void accept(AstNodeVisitor &p_visitor) override;

    void print() override;

  private:
    // TODO: expression, compound statement
    AstNode *condition;
    AstNode *body;
};

#endif


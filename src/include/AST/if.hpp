#ifndef __AST_IF_NODE_H
#define __AST_IF_NODE_H

#include "AST/ast.hpp"

class IfNode : public AstNode {
  public:
    IfNode(const uint32_t line, const uint32_t col, AstNode *p_cond, AstNode *p_body, AstNode *p_body_el
           /* TODO: expression, compound statement, compound statement */);
    ~IfNode() = default;

    void visitChildNodes(AstNodeVisitor &p_visitor);
    void accept(AstNodeVisitor &p_visitor) override;


    void print() override;

  private:
    // TODO: expression, compound statement, compound statement
    AstNode *condition;
    AstNode *body;
    AstNode *body_else;
};

#endif


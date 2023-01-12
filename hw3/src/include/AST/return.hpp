#ifndef __AST_RETURN_NODE_H
#define __AST_RETURN_NODE_H

#include "AST/ast.hpp"

class ReturnNode : public AstNode {
  public:
    ReturnNode(const uint32_t line, const uint32_t col, AstNode *p_return_var
               /* TODO: expression */);
    ~ReturnNode() = default;

    void visitChildNodes(AstNodeVisitor &p_visitor);
    void accept(AstNodeVisitor &p_visitor) override;


    void print() override;

  private:
    // TODO: expression
    AstNode *return_value;
};

#endif


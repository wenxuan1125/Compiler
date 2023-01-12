#ifndef __AST_READ_NODE_H
#define __AST_READ_NODE_H

#include "AST/ast.hpp"

class ReadNode : public AstNode {
  public:
    ReadNode(const uint32_t line, const uint32_t col, AstNode *p_var_ref
             /* TODO: variable reference */);
    ~ReadNode() = default;
    void visitChildNodes(AstNodeVisitor &p_visitor);
    void accept(AstNodeVisitor &p_visitor) override;


    void print() override;

  private:
    // TODO: variable reference
    AstNode *target;
};

#endif


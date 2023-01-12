#ifndef __AST_ASSIGNMENT_NODE_H
#define __AST_ASSIGNMENT_NODE_H

#include "AST/ast.hpp"

class AssignmentNode : public AstNode {
  public:
    AssignmentNode(const uint32_t line, const uint32_t col, AstNode *p_lval, AstNode *p_expr
                   /* TODO: variable reference, expression */);
    ~AssignmentNode() = default;
    
    void visitChildNodes(AstNodeVisitor &p_visitor);
    void accept(AstNodeVisitor &p_visitor) override;

    void print() override;

  private:
    // TODO: variable reference, expression
    AstNode *lvalue;
    AstNode *expression;
};

#endif


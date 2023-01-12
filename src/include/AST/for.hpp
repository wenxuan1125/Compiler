#ifndef __AST_FOR_NODE_H
#define __AST_FOR_NODE_H

#include "AST/ast.hpp"

class ForNode : public AstNode {
  public:
    ForNode(const uint32_t line, const uint32_t col,
           AstNode *p_loop_var, AstNode *p_init, AstNode *p_cond, AstNode *p_body
            /* TODO: declaration, assignment, expression,
             *       compound statement */);
    ~ForNode() = default;

    void visitChildNodes(AstNodeVisitor &p_visitor);
    void accept(AstNodeVisitor &p_visitor) override;


    void print() override;

  private:
    // TODO: declaration, assignment, expression, compound statement
    AstNode *loop_variable;
    AstNode *initial;
    AstNode *condition;
    AstNode *body;
    
};

#endif


#ifndef __AST_RETURN_NODE_H
#define __AST_RETURN_NODE_H

#include "AST/ast.hpp"
#include "AST/expression.hpp"

#include <memory>
#include <cstring>

class ReturnNode : public AstNode {
  public:
    ReturnNode(const uint32_t line, const uint32_t col,
               ExpressionNode *p_retval);
    ~ReturnNode() = default;

    void accept(AstNodeVisitor &p_visitor) override;
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
    const int checkInvalidChildren() const;
    const char *getReturnTypeCString() const;
    const uint32_t getReturnLocationCol() const; 

  private:
    std::unique_ptr<ExpressionNode> retval;
};

#endif


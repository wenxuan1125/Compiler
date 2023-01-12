#ifndef __AST_WHILE_NODE_H
#define __AST_WHILE_NODE_H

#include "AST/CompoundStatement.hpp"
#include "AST/ast.hpp"
#include "AST/expression.hpp"

#include <memory>
#include <cstring>

class WhileNode : public AstNode {
  public:
    WhileNode(const uint32_t line, const uint32_t col,
              ExpressionNode *p_condition, CompoundStatementNode *p_body);
    ~WhileNode() = default;

    void accept(AstNodeVisitor &p_visitor) override;
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
    const int checkInvalidCondition() const;
    const int checkConditionBoolType() const;
    const uint32_t getConditionLocationCol() const;

  private:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<CompoundStatementNode> body;
};

#endif


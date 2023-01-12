#ifndef __AST_ASSIGNMENT_NODE_H
#define __AST_ASSIGNMENT_NODE_H

#include "AST/VariableReference.hpp"
#include "AST/ast.hpp"
#include "AST/expression.hpp"
#include "AST/ConstantValue.hpp"

#include <memory>

class AssignmentNode : public AstNode {
  public:
    AssignmentNode(const uint32_t line, const uint32_t col,
                   VariableReferenceNode *p_lvalue, ExpressionNode *p_expr);
    ~AssignmentNode() = default;

    void accept(AstNodeVisitor &p_visitor) override;
    const int checkInvalidLvalue() const;
    const int checkInvalidRvalue() const;
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
    const char *getLvalueNameCString() const;
    const uint32_t getLvalueLocationCol() const;
    const uint32_t getRvalueLocationCol() const;
    const char *getRvalueTypeCString() const;
    const char *getLvalueTypeCString() const;
    const int checkLvalueScalarType() const;
    const int checkRvalueScalarType() const;
    const int checkCompatibleLvalueRvalueType() const;
    const char *getConstantValueCString();

  private:
    std::unique_ptr<VariableReferenceNode> lvalue;
    std::unique_ptr<ExpressionNode> expr;
    //std::shared_ptr<ExpressionNode> expr;
};

#endif


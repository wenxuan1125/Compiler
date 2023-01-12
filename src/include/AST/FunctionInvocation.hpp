#ifndef __AST_FUNCTION_INVOCATION_NODE_H
#define __AST_FUNCTION_INVOCATION_NODE_H

#include "AST/expression.hpp"

#include <memory>
#include <vector>
#include <cstring>

class FunctionInvocationNode : public ExpressionNode {
  public:
    typedef std::vector<std::unique_ptr<ExpressionNode>> Exprs;

    FunctionInvocationNode(const uint32_t line, const uint32_t col,
                           const char *p_name, Exprs *p_arguments);
    ~FunctionInvocationNode() = default;

    const char *getNameCString() const;
    const char *getArgumentTypeCString( const int id) const;
    const char *getParameterTypeCString( const int id, const char *parameters_type) const;
    const uint32_t getArgumentLocationCol( const int id) const; 
    const int getArgumentsNum() const; 
    const int checkInvalidChildren() const;
    const int checkArgumentsType(const char *parameters_type) const;

    void accept(AstNodeVisitor &p_visitor) override;
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
    //void setNodeType(PTypeSharedPtr p_type);

  private:
    const std::string name;
    Exprs arguments;
};

#endif


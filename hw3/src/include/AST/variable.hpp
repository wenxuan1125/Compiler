#ifndef __AST_VARIABLE_NODE_H
#define __AST_VARIABLE_NODE_H

#include "AST/ast.hpp"

class VariableNode : public AstNode {
  public:
    VariableNode(const uint32_t line, const uint32_t col, 
                 /* TODO: variable name, type, constant value */
                 const char *p_name, const char *p_type,  
                 AstNode *p_constant);
    ~VariableNode() = default;

    void visitChildNodes(AstNodeVisitor &p_visitor);
    void accept(AstNodeVisitor &p_visitor) override;
    const char * getTypeCString() const;
    const char * getNameCString() const;

    void print() override;

  private:
    // TODO: variable name, type, constant value
    const std::string name;
    const std::string type;
    AstNode *constant;  // a constant value node
};

#endif


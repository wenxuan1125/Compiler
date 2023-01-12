#ifndef __AST_FUNCTION_NODE_H
#define __AST_FUNCTION_NODE_H

#include "AST/ast.hpp"

class FunctionNode : public AstNode {
  public:
    FunctionNode(const uint32_t line, const uint32_t col,
                 /* TODO: name, declarations, return type,
                  *       compound statement (optional) */
                const char *p_name, const char *p_return,
                std::vector<AstNode*> *p_decl_list, std::vector<char *> *p_decl_type,
                AstNode *p_body);
    ~FunctionNode() = default;

    void visitChildNodes(AstNodeVisitor &p_visitor);
    void accept(AstNodeVisitor &p_visitor) override;
    const char *getNameCString() const;
    const char *getReturnTypeCString() const;
    const char *getPrototypeCString() const;

    void print() override;

  private:
    // TODO: name, declarations, return type, compound statement
    const std::string name;
    const std::string return_type;
    std::vector<AstNode*> *declaration_list;  // list of declation nodes
    std::vector<char *> *declaration_type;  // for function prototype
    AstNode *body; // body is a compound statement node
};

#endif


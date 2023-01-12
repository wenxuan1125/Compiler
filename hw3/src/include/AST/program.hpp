#ifndef __AST_PROGRAM_NODE_H
#define __AST_PROGRAM_NODE_H

#include "AST/ast.hpp"

class ProgramNode : public AstNode {
  public:
    ProgramNode(const uint32_t line, const uint32_t col,
                const char *p_name, const char *p_return, 
                std::vector<AstNode*> *p_decl_list, std::vector<AstNode*> *p_func_list, 
                AstNode *p_body
                );
    ~ProgramNode() = default;

    // visitor pattern version: const char *getNameCString() const;
    void visitChildNodes(AstNodeVisitor &p_visitor);
    const char *getNameCString() const;
    void accept(AstNodeVisitor &p_visitor) override;

    void print() override;

  private:
    const std::string name;
    // TODO: return type, declarations, functions, compound statement
    const std::string return_type;
    std::vector<AstNode*> *declation_list;  // list of declation nodes
    std::vector<AstNode*> *function_list;   // list of function nodes
    AstNode *body;  // body is a compound statement node
};

#endif


#ifndef __AST_COMPOUND_STATEMENT_NODE_H
#define __AST_COMPOUND_STATEMENT_NODE_H

#include "AST/ast.hpp"

class CompoundStatementNode : public AstNode {
  public:
    CompoundStatementNode(const uint32_t line, const uint32_t col,
                          /* TODO: declarations, statements */
                          std::vector<AstNode*> *p_decl_list, 
                          std::vector<AstNode*> *p_stat_list);
    ~CompoundStatementNode() = default;

    void visitChildNodes(AstNodeVisitor &p_visitor);
    void accept(AstNodeVisitor &p_visitor) override;


    void print() override;

  private:
    // TODO: declarations, statements
    std::vector<AstNode*> *declaration_list;  // list of declaration nodes
    std::vector<AstNode*> *statement_list;   // list of statement nodes
};

#endif


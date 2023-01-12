#include "AST/CompoundStatement.hpp"

// TODO
CompoundStatementNode::CompoundStatementNode(const uint32_t line,
                                             const uint32_t col, 
                                             std::vector<AstNode*> *p_decl_list, 
                                             std::vector<AstNode*> *p_stat_list )
    : AstNode{line, col}, declaration_list(p_decl_list), statement_list(p_stat_list) {}

// TODO: You may use code snippets in AstDumper.cpp
void CompoundStatementNode::print() {

    AstNode::outputIndentationSpace(m_indentation);
    std::printf("compound statement <line: %u, col: %u>\n",
                location.line,
                location.col);    
    AstNode::incrementIndentation();
    if( declaration_list!= NULL){
        for(auto &declaration_list: *declaration_list)
            declaration_list->print();
    }
    if( statement_list!= NULL){
        for(auto &statement_list: *statement_list)
            statement_list->print();
    }
    AstNode::decrementIndentation();

}


// void CompoundStatementNode::visitChildNodes(AstNodeVisitor &p_visitor) {
//     // TODO
// }

void CompoundStatementNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void CompoundStatementNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if (declaration_list != NULL)
    {
        for(auto &declaration_list: *declaration_list)
            declaration_list->accept(p_visitor);
    }
    if( statement_list!= NULL){
        for(auto &statement_list: *statement_list)
            statement_list->accept(p_visitor);
    }
}


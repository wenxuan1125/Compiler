#include "AST/decl.hpp"

// TODO
DeclNode::DeclNode(const uint32_t line, const uint32_t col,
                    std::vector<AstNode*> *p_var_list)
    : AstNode{line, col}, variable_list(p_var_list) {}

// TODO
//DeclNode::DeclNode(const uint32_t line, const uint32_t col)
//    : AstNode{line, col} {}

// TODO: You may use code snippets in AstDumper.cpp
void DeclNode::print() {

    AstNode::outputIndentationSpace(m_indentation);
    std::printf("declaration <line: %u, col: %u>\n",
                location.line, location.col);
    AstNode::incrementIndentation();
    
    if (variable_list != NULL)
    {
        for(auto &variable_list: *variable_list)
            variable_list->print();
    }
    
    AstNode::decrementIndentation();
}

// void DeclNode::visitChildNodes(AstNodeVisitor &p_visitor) {
//     // TODO
// }
void DeclNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void DeclNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if (variable_list != NULL)
    {
        for(auto &variable_list: *variable_list)
            variable_list->accept(p_visitor);
    }
}


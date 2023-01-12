#include "AST/program.hpp"

// TODO
ProgramNode::ProgramNode(const uint32_t line, const uint32_t col,
                         const char *p_name, const char *p_return,
                         std::vector<AstNode*> *p_decl_list, std::vector<AstNode*> *p_func_list, 
                         AstNode *p_body)
    : AstNode{line, col}, name(p_name), return_type(p_return), 
    declation_list(p_decl_list), function_list(p_func_list), body(p_body) {}

// visitor pattern version: const char *ProgramNode::getNameCString() const { return name.c_str(); }

void ProgramNode::print() {
    // TODO
    //outputIndentationSpace(m_indentation);
    AstNode::outputIndentationSpace(m_indentation);
    std::printf("program <line: %u, col: %u> %s %s\n",
                location.line, location.col,
                name.c_str(), "void");
                
    AstNode::incrementIndentation();
    if (declation_list != NULL)
    {
        for(auto &declation_list: *declation_list)
            declation_list->print();
    }
    if( function_list!= NULL){
        for(auto &function_list: *function_list)
            function_list->print();
    }
    if( body!= NULL)
        body->print();
    AstNode::decrementIndentation();
    // for (int i = 0; i < declation_list->size(); i++)
    //     declation_list[i].print();
    // for (int i = 0; i < function_list->size(); i++)
    //     function_list[i].print();
    //visitChildNodes();
    ///decrementIndentation(m_indentation);

}

const char *ProgramNode::getNameCString() const { return name.c_str(); }
void ProgramNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void ProgramNode::visitChildNodes(AstNodeVisitor &p_visitor) { // visitor pattern version
    /* TODO
     *
     * for (auto &decl : var_decls) {
     *     decl->accept(p_visitor);
     * }
     *
     * // functions
     *
     * body->accept(p_visitor);
     */

    if (declation_list != NULL)
    {
        for(auto &declation_list: *declation_list)
            declation_list->accept(p_visitor);
    }
    if( function_list!= NULL){
        for(auto &function_list: *function_list)
            function_list->accept(p_visitor);
    }
    if( body!= NULL)
        body->accept(p_visitor);
}


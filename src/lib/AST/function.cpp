#include "AST/function.hpp"

// TODO
static char s[100];
FunctionNode::FunctionNode(const uint32_t line, const uint32_t col,
                         const char *p_name, const char *p_return,
                         std::vector<AstNode*> *p_decl_list, std::vector<char *> *p_decl_type, 
                         AstNode *p_body)
    : AstNode{line, col}, name(p_name), return_type(p_return), 
    declaration_list(p_decl_list), declaration_type(p_decl_type), body(p_body) {}

// TODO: You may use code snippets in AstDumper.cpp
const char *FunctionNode::getNameCString() const { return name.c_str(); }
const char *FunctionNode::getReturnTypeCString() const{ return return_type.c_str(); }
const char *FunctionNode::getPrototypeCString() const {
    std::strcpy(s, "");
    std::strcat(s, "(");
    if (declaration_type != NULL)
    {
        // function prototype
        for (int i = 0; i < declaration_type->size(); i++){
            if (i != declaration_type->size() - 1){
                std::strcat(s, declaration_type->at(i));
                std::strcat(s, ", ");

            }
            else
                std::strcat(s, declaration_type->at(i));
        }
    }

    std::strcat(s, ")");

    //std::printf("%s", s);
    return s;
}
void FunctionNode::print() {
    AstNode::outputIndentationSpace(m_indentation);
    std::printf("function declaration <line: %u, col: %u> %s %s (",
                location.line, location.col,
                name.c_str(), return_type.c_str());

    if (declaration_type != NULL)
    {
        // function prototype
        for (int i = 0; i < declaration_type->size(); i++){
            if( i!=declaration_type->size()-1)
                printf("%s, ", declaration_type->at(i));
            else
                printf("%s", declaration_type->at(i));
        }
            
    }
    printf(")\n");
                
    AstNode::incrementIndentation();
    //printf("  ");
    if (declaration_list != NULL)
    {
        for(auto &declaration_list: *declaration_list)
            declaration_list->print();
    }
    if( body!= NULL)
        body->print();
    AstNode::decrementIndentation();
}

// void FunctionNode::visitChildNodes(AstNodeVisitor &p_visitor) {
//     // TODO
// }
void FunctionNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void FunctionNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if (declaration_list != NULL)
    {
        for(auto &declaration_list: *declaration_list)
            declaration_list->accept(p_visitor);
    }
    if( body!= NULL)
        body->accept(p_visitor);
}


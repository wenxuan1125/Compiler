#include "AST/variable.hpp"

// TODO
VariableNode::VariableNode(const uint32_t line, const uint32_t col, 
                         const char *p_name, const char *p_type,  
                         AstNode *p_constant)
    : AstNode{line, col}, name(p_name), type(p_type), constant(p_constant) {}

// TODO: You may use code snippets in AstDumper.cpp
void VariableNode::print() {
    AstNode::outputIndentationSpace(m_indentation);
    std::printf("variable <line: %u, col: %u> %s %s\n",
                location.line, location.col,
                name.c_str(), type.c_str());

    AstNode::incrementIndentation();
    if (constant != NULL)
    {
        constant->print();
    }
    AstNode::decrementIndentation();
}

const char *VariableNode::getTypeCString() const { return type.c_str(); }
const char *VariableNode::getNameCString() const { return name.c_str(); }

// void VariableNode::visitChildNodes(AstNodeVisitor &p_visitor) {
//     // TODO
// }
void VariableNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void VariableNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if( constant!= NULL)
        constant->accept(p_visitor);
}


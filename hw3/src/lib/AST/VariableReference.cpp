#include "AST/VariableReference.hpp"

// TODO
VariableReferenceNode::VariableReferenceNode(const uint32_t line,
                                             const uint32_t col,
                                    const char *p_name, std::vector<AstNode*> *p_expr_list)
    : ExpressionNode{line, col}, name(p_name), expression_list(p_expr_list) {}

// TODO
// VariableReferenceNode::VariableReferenceNode(const uint32_t line,
//                                              const uint32_t col)
//     : ExpressionNode{line, col} {}

// TODO: You may use code snippets in AstDumper.cpp
void VariableReferenceNode::print() {
    AstNode::outputIndentationSpace(m_indentation);
    std::printf("variable reference <line: %u, col: %u> %s\n",
                location.line, location.col, name.c_str());
                
    AstNode::incrementIndentation();
    if (expression_list != NULL)
    {
        for(auto &expression_list: *expression_list)
            expression_list->print();
    }
    AstNode::decrementIndentation();
}

// void VariableReferenceNode::visitChildNodes(AstNodeVisitor &p_visitor) {
//     // TODO
// }
const char *VariableReferenceNode::getNameCString() const { return name.c_str(); }
void VariableReferenceNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void VariableReferenceNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if (expression_list != NULL)
    {
        for(auto &expression_list: *expression_list)
            expression_list->accept(p_visitor);
    }
}

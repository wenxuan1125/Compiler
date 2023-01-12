#include "AST/FunctionInvocation.hpp"

// TODO
FunctionInvocationNode::FunctionInvocationNode(const uint32_t line, const uint32_t col,
                                    const char *p_name, std::vector<AstNode*> *p_expr_list)
    : ExpressionNode{line, col}, name(p_name), expression_list(p_expr_list) {}

// TODO: You may use code snippets in AstDumper.cpp
void FunctionInvocationNode::print() {
    AstNode::outputIndentationSpace(m_indentation);
    std::printf("function invocation <line: %u, col: %u> %s\n",
                location.line, location.col, name.c_str());
                
    AstNode::incrementIndentation();
    if (expression_list != NULL)
    {
        for(auto &expression_list: *expression_list)
            expression_list->print();
    }
    AstNode::decrementIndentation();
}

// void FunctionInvocationNode::visitChildNodes(AstNodeVisitor &p_visitor) {
//     // TODO
// }

const char *FunctionInvocationNode::getNameCString() const { return name.c_str(); }
void FunctionInvocationNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void FunctionInvocationNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if (expression_list != NULL)
    {
        for(auto &expression_list: *expression_list)
            expression_list->accept(p_visitor);
    }
}


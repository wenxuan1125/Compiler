#include "AST/expression.hpp"

ExpressionNode::ExpressionNode(const uint32_t line, const uint32_t col)
    : AstNode{line, col} {}

const char *ExpressionNode::getPTypeCString() const{
    if( type!=nullptr){
        return type->getPTypeCString();
    }
    else{
        return "null";
    }
}

void ExpressionNode::setNodeType(PTypeSharedPtr p_type){
    type = p_type;
}

void ExpressionNode::setNodeTypeDimensions(std::vector<uint64_t> dims){
    type->setDimensions(dims);
}


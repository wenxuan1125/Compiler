#include "AST/ConstantValue.hpp"

// TODO
ConstantValueNode::ConstantValueNode(const uint32_t line, const uint32_t col,
                      /* TODO: constant value */
                      Constant_Value p_constant_value)
    : ExpressionNode{line, col}, constant_value(p_constant_value) {}

// TODO: You may use code snippets in AstDumper.cpp
void ConstantValueNode::print() {
    AstNode::outputIndentationSpace(m_indentation);
    if( constant_value.int_type){
        std::printf("constant <line: %u, col: %u> %d\n",
                location.line, location.col,
                constant_value.int_value);
    }
    else if( constant_value.real_type){
        std::printf("constant <line: %u, col: %u> %f\n",
                location.line, location.col,
                constant_value.real_value);
    }
    else if( constant_value.str_type || constant_value.bool_type){
        std::printf("constant <line: %u, col: %u> %s\n",
                location.line, location.col,
                constant_value.str_value);
    }

}
const char *ConstantValueNode::getStringValue() const { return constant_value.str_value; }
const int ConstantValueNode::getIntValue() const { return constant_value.int_value; }
const float ConstantValueNode::getRealValue() const { return constant_value.real_value; }
const int ConstantValueNode::getConstantType() const { 
    if( constant_value.int_type){
        return 1;
    }
    else if( constant_value.real_type){
        return 2;
    }
    else if( constant_value.str_type || constant_value.bool_type){
        return 3;
    }


}
void ConstantValueNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}
void ConstantValueNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    
}

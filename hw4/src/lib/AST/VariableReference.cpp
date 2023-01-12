#include "AST/VariableReference.hpp"
#include "visitor/AstNodeVisitor.hpp"

VariableReferenceNode::VariableReferenceNode(const uint32_t line,
                                             const uint32_t col,
                                             const char *p_name)
    : ExpressionNode{line, col}, name(p_name) {
    
    


}

VariableReferenceNode::VariableReferenceNode(const uint32_t line,
                                             const uint32_t col,
                                             const char *p_name,
                                             Exprs *p_indices)
    : ExpressionNode{line, col}, name(p_name), indices(std::move(*p_indices)) {}

const char *VariableReferenceNode::getNameCString() const {
    return name.c_str();
}
const int VariableReferenceNode::getIndicesNum() const{
    return indices.size();
}

const int VariableReferenceNode::checkNonIntegerIndices() const{
    for (int i = 0; i < indices.size(); i++){
        if( std::strcmp(indices[i]->getPTypeCString(), "integer")!=0){

            return indices[i]->getLocation().col;
        }

            
    }
    return -1;
}

void VariableReferenceNode::accept(AstNodeVisitor &p_visitor) {
    p_visitor.visit(*this);
}

void VariableReferenceNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    for (auto &index : indices) {
        index->accept(p_visitor);
    }
}

const int VariableReferenceNode::checkInvalidChildren() const{
    for (int i = 0; i < indices.size(); i++){
        if( std::strcmp(indices[i]->getPTypeCString(), "null")==0){

            return 1;
        }
    }

    return 0;
}


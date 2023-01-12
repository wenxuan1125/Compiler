#include "AST/read.hpp"
#include "visitor/AstNodeVisitor.hpp"

ReadNode::ReadNode(const uint32_t line, const uint32_t col,
                   VariableReferenceNode *p_target)
    : AstNode{line, col}, target(p_target) {}

void ReadNode::accept(AstNodeVisitor &p_visitor) { p_visitor.visit(*this); }

void ReadNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    target->accept(p_visitor);
}

const int ReadNode::checkInvalidChildren() const{

    if(std::strcmp(target->getPTypeCString(), "null")==0)
        return 1;

    return 0;
}
const uint32_t ReadNode::getTargetLocationCol() const{
    return target->getLocation().col;
} 

const int ReadNode::checkTargetScalarType() const{
    if(std::strcmp(target->getPTypeCString(), "integer")==0 || std::strcmp(target->getPTypeCString(), "real")==0
    || std::strcmp(target->getPTypeCString(), "string")==0 || std::strcmp(target->getPTypeCString(), "boolean")==0)
        return 1;
    else
        return 0;
}
const char *ReadNode::getTargetNameCString() const{
    return target->getNameCString();
}


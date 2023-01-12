#include <AST/ast.hpp>

AstNode::AstNode(const uint32_t line, const uint32_t col)
    : location(line, col) {}

// prevent the linker from complaining
AstNode::~AstNode() {}

const Location &AstNode::getLocation() const { return location; }

uint32_t AstNode::m_indentation = 0u;

void AstNode::outputIndentationSpace(const uint32_t indentation) {
    std::printf("%*s", indentation, "");
}

void AstNode::incrementIndentation() {
    m_indentation += m_indentation_stride;
}

void AstNode::decrementIndentation() {
    m_indentation -= m_indentation_stride;
}


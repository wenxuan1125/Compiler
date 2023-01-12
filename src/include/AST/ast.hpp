#ifndef __AST_H
#define __AST_H

#include <cstdint>
#include <string>
#include <vector> 
#include <cstring>
#include "visitor/AstNodeVisitor.hpp"

struct Location {
    Location(const uint32_t line, const uint32_t col) : line(line), col(col) {}

    uint32_t line;
    uint32_t col;
};

struct Constant_Value {

    uint32_t line;
    uint32_t col;
    int int_value;
    float real_value;
    char *str_value;  // string or bool
    bool int_type = false;
    bool real_type = false;
    bool str_type = false;
    bool bool_type = false;

};
struct Name {

    char *id;
    uint32_t line;
    uint32_t col;

};

struct Binary_Op {
  bool mul = false;
  bool div = false;
  bool mod = false;
  bool add = false;
  bool sub = false;
  bool lt = false;
  bool le = false;
  bool eq = false;
  bool ne = false;
  bool ge = false;
  bool gt = false;
  bool AND = false;
  bool OR = false;
};

struct Unary_Op {
  bool neg = false;
  bool NOT = false;
};

class AstNode
{
public:
  AstNode(const uint32_t line, const uint32_t col);
  virtual ~AstNode() = 0;

  const Location &getLocation() const;
  virtual void print() = 0;
  virtual void accept(AstNodeVisitor &p_visitor) = 0;

  static void outputIndentationSpace(const uint32_t indentation);
  static void incrementIndentation();
  static void decrementIndentation();

protected:
  const Location location;
  static const uint32_t m_indentation_stride = 2u;
  static uint32_t m_indentation;
};

struct Func_Prototype {

  std::vector<AstNode*> *decl_list;
  std::vector<char*> *type_list;

};



#endif


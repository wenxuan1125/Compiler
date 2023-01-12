#ifndef __SEMA_SEMANTIC_ANALYZER_H
#define __SEMA_SEMANTIC_ANALYZER_H

#include "visitor/AstNodeVisitor.hpp"
#include <iostream>
#include <stack>
#include <vector>
#include <string>
#include <cstring>

enum KindType{
  program, 
  function, 
  parameter, 
  variable, 
  loop_var, 
  constant
};

class SymbolEntry {
  public:
    SymbolEntry( const char *p_name, const KindType p_kind, const int p_level, const char *p_type, const char *p_attri)
    : name(p_name), kind(p_kind), level(p_level), type(p_type), attribute(p_attri){}
    void dumpEntry(void){
      printf("%-33s", name.c_str());

      if( kind==program)
        printf("%-11s", "program");
      else if( kind==function)
        printf("%-11s", "function");
      else if( kind==parameter)
        printf("%-11s", "parameter");
      else if( kind==variable)
        printf("%-11s", "variable");
      else if( kind==loop_var)
        printf("%-11s", "loop_var");
      else if( kind==constant)
        printf("%-11s", "constant");
      //printf("%-11s", kind.c_str());
      
      if( level==0)
        printf("%d%-10s", 0, "(global)");
      else 
        printf("%d%-10s", level, "(local)");

      printf("%-17s", type.c_str());
      printf("%-11s", attribute.c_str());
      puts("");
    }

    void changeKindValue( const KindType p_kind){
      kind = p_kind;
    }
    void changeAttributeValue( const char *p_attri){
      attribute = p_attri;
    }
    const char *getNameCString() const { return name.c_str(); }
    const char *getTypeCString() const { return type.c_str(); }
    const char *getAttributeCString() const { return attribute.c_str(); }
    const KindType getKindValue() const { return kind; }
    const int getTypeDimension() const {
      int dim = 0;
      for (int i = 0; i < type.size(); i++)
      {
        if(type[i]=='[')
          dim++;
      }
      return dim;
    }
    std::vector<uint64_t> getNewTypeDimensions( int ignore) const {
      std::vector<uint64_t> dims;
      int count = 0;
      for (int i = 0; i < type.size(); i++)
      {
        if(type[i]=='['){

          if(count!=ignore){
            count++;
            continue;
          }

          uint64_t d = 0;
          i++;
          while (type[i] != ']')
          {
            d = d * 10;
            d = d + (type[i] - '0');
            i++;
          }

          
          dims.push_back(d);
        }
      }
      return dims;
    }

    const int getFunctionParameterNum() const{
      int num;
      if( attribute.size()==0)
        num = 0;
      else
        num = 1;

      for (int i = 0; i < attribute.size(); i++)
      {
        if(attribute[i]==',')
          num++;
      }
      return num;
    }

  private:
      // Variable names
    std::string name;
    // Kind
    KindType kind;
    //std::string kind;
    // Level
    int level;
    // Type
    std::string type;
    // Attribute
    std::string attribute;
};

class SymbolTable {
  public:
    SymbolTable(){}
    
    void addSymbol(SymbolEntry *entry){
      //printf("****%-11s\n", p_attri);
      //SymbolEntry *entry = new SymbolEntry( p_name, p_kind, p_level, p_type, p_attri);
      entries.push_back( entry);
    }
    void dumpDemarcation(const char chr){
      for (size_t i = 0; i < 110; ++i) {
        printf("%c", chr);
      }
      puts("");
    }
    void dumpSymbol(void){
      dumpDemarcation('=');
      printf("%-33s%-11s%-11s%-17s%-11s\n", "Name", "Kind", "Level", "Type",
                                            "Attribute");
      dumpDemarcation('-');
      {
        for( int i=0; i<entries.size(); i++){
          entries[i]->dumpEntry();
        }
        /*printf("%-33s", "func");
        printf("%-11s", "function");
        printf("%d%-10s", 0, "(global)");
        printf("%-17s", "boolean");
        printf("%-11s", "integer, real [2][3]");
        puts("");*/
      }
      dumpDemarcation('-');
    }

    const int checkRedeclaration( const char *p_name) const {
      //std::printf("*******\n");
      for (int i = 0; i < entries.size(); i++)
      {
        if (std::strcmp(entries[i]->getNameCString(), p_name)==0)
          return 1;
        //else
          //std::printf("%s %s\n", p_name, entries[i]->getNameCString());
      }
      //std::printf("*******\n");
      return 0;
    }
    const int checkInErrorDeclaration( const char *p_name) const {
      for (int i = 0; i < error_declaration.size(); i++)
      {
        if (std::strcmp(error_declaration[i], p_name)==0)
          return 1;
      }
      return 0;
    }

    const SymbolEntry *getEntry(const char *p_name) const { 
      for (int i = 0; i < entries.size(); i++)
      {
        if (std::strcmp(entries[i]->getNameCString(), p_name)==0)
          return entries[i];
        //else
        //std::printf("%s %s\n", p_name, entries[i]->getNameCString());
      }

      return NULL;
    }

    const SymbolEntry *getEntryWithId(const int id) const { 
      return entries[id];
    }
    const int checkFunctionInTable() const{
      for (int i = entries.size() - 1; i >= 0; i--){
        if(entries[i]->getKindValue()==function){
          return i;
        }
      }

      return -1;
    }

    void addErrorSymbol(const char *name){
      //printf("****%-11s\n", p_attri);
      //SymbolEntry *entry = new SymbolEntry( p_name, p_kind, p_level, p_type, p_attri);
      error_declaration.push_back( name);
    }


    // other methods
  private:
    std::vector<SymbolEntry *> entries;
    std::vector< const char *> error_declaration;
};

class SymbolManager {
  public:
    SymbolManager(){}
    void pushScope(SymbolTable *new_scope){
      tables.push(new_scope);
    }
    void popScope(){
      tables.pop();
    }
    SymbolTable *topScope(){
      return tables.top();
    } 
    int getScopeSize(){
      return tables.size();
    }
    // other methods
  private:
    std::stack <SymbolTable *> tables;
};

class SemanticAnalyzer : public AstNodeVisitor {
  public:
    SemanticAnalyzer() = default;
    ~SemanticAnalyzer() = default;

    void visit(ProgramNode &p_program) override;
    void visit(DeclNode &p_decl) override;
    void visit(VariableNode &p_variable) override;
    void visit(ConstantValueNode &p_constant_value) override;
    void visit(FunctionNode &p_function) override;
    void visit(CompoundStatementNode &p_compound_statement) override;
    void visit(PrintNode &p_print) override;
    void visit(BinaryOperatorNode &p_bin_op) override;
    void visit(UnaryOperatorNode &p_un_op) override;
    void visit(FunctionInvocationNode &p_func_invocation) override;
    void visit(VariableReferenceNode &p_variable_ref) override;
    void visit(AssignmentNode &p_assignment) override;
    void visit(ReadNode &p_read) override;
    void visit(IfNode &p_if) override;
    void visit(WhileNode &p_while) override;
    void visit(ForNode &p_for) override;
    void visit(ReturnNode &p_return) override;

  private:
    // TODO: something like symbol manager (manage symbol tables)
    //       context manager, return type manager
    
    int m_level = 0;
};

#endif


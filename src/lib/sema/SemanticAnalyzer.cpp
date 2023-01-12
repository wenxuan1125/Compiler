#include "sema/SemanticAnalyzer.hpp"
#include "visitor/AstNodeInclude.hpp"
#include "AST/PType.hpp"
#define MAX_LINE_NUM        201


extern int32_t opt_dump;
extern char *source_code[MAX_LINE_NUM + 1];
SymbolManager *symbol_manager = new SymbolManager;
SymbolTable *current_table;
SymbolEntry *current_entry;
SymbolManager *temp_manager = new SymbolManager;
SymbolTable *temp_current;
std::vector<const char *> loop_variable;
bool flag_for = false;
bool set_loop_var = false;
bool flag_function = false;
bool flag_var = false;
bool flag_parameter = false;
bool in_function = false;
extern bool error_happen;

void showArrow(int pos){
    for (int i = 0; i < pos + 3; i++)
        std::cerr << " ";
    std::cerr << "^\n";
    
}
int checkLoopVarRedeclaration(const char *p_name){
    for (int i = 0; i < loop_variable.size(); i++){
        if (std::strcmp(loop_variable[i], p_name)==0)
            return 1;
    }
    return 0;
}

void SemanticAnalyzer::visit(ProgramNode &p_program) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    current_table=new SymbolTable;
    current_entry = new SymbolEntry(p_program.getNameCString(), program, m_level, "void", "");
    current_table->addSymbol(current_entry);
    symbol_manager->pushScope(current_table);

    p_program.visitChildNodes(*this);

    current_table=symbol_manager->topScope();
    if(opt_dump)
        current_table->dumpSymbol();
    symbol_manager->popScope();

  
}

void SemanticAnalyzer::visit(DeclNode &p_decl) {
    p_decl.visitChildNodes(*this);
}

void SemanticAnalyzer::visit(VariableNode &p_variable) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    current_table = symbol_manager->topScope();

    
    if(current_table->checkRedeclaration(p_variable.getNameCString())){
        error_happen = true;
        std::cerr << "<Error> Found in line " << p_variable.getLocation().line
                  << ", column " << p_variable.getLocation().col << ": symbol '" << p_variable.getNameCString()
                  << "' is redeclared\n";
        // show source code
        std::cerr << "    " << source_code[p_variable.getLocation().line] << '\n';
        showArrow(p_variable.getLocation().col);
        
    }
    else if( checkLoopVarRedeclaration(p_variable.getNameCString())){
        // inner for loop
        error_happen = true;
        std::cerr << "<Error> Found in line " << p_variable.getLocation().line 
            << ", column " << p_variable.getLocation().col << ": symbol '" << p_variable.getNameCString()
            << "' is redeclared\n";
        // show source code
        std::cerr << "    " << source_code[p_variable.getLocation().line] << '\n';
        showArrow(p_variable.getLocation().col);
        
    }
    else{
        if(flag_parameter)
            current_entry = new SymbolEntry(p_variable.getNameCString(), parameter, m_level, p_variable.getTypeCString(), "");
        else
            current_entry = new SymbolEntry(p_variable.getNameCString(), variable, m_level, p_variable.getTypeCString(), "");
        current_table->addSymbol(current_entry);
        flag_var = true;
        p_variable.visitChildNodes(*this);
        flag_var = false;

    }


    if(strchr( p_variable.getTypeCString(), '[')!=NULL){
        
        if( p_variable.checkInvalidDimensions()){
            // invalid array dimension
            error_happen = true;
            std::cerr << "<Error> Found in line " << p_variable.getLocation().line 
                << ", column " << p_variable.getLocation().col << ": '" << p_variable.getNameCString()
                << "' declared as an array with an index that is not greater than 0\n";
            // show source code
            std::cerr << "    " << source_code[p_variable.getLocation().line] << '\n';
            showArrow(p_variable.getLocation().col);
            current_table->addErrorSymbol(p_variable.getNameCString());
        }
    }
    
    

}

void SemanticAnalyzer::visit(ConstantValueNode &p_constant_value) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    
    if( flag_for && !flag_var){

        current_entry->changeKindValue(loop_var);
        flag_for = false;
        loop_variable.push_back(current_entry->getNameCString());

    }
    else if( flag_var){
        current_entry->changeKindValue(constant);
        current_entry->changeAttributeValue(p_constant_value.getConstantValueCString());

    
    }

}

void SemanticAnalyzer::visit(FunctionNode &p_function) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */


    current_table=symbol_manager->topScope();

    if(current_table->checkRedeclaration(p_function.getNameCString())){
        error_happen = true;
        std::cerr << "<Error> Found in line " << p_function.getLocation().line 
                << ", column " << p_function.getLocation().col << ": symbol '" << p_function.getNameCString()
                << "' is redeclared\n";
        std::cerr << "    " << source_code[p_function.getLocation().line] << '\n';
        showArrow(p_function.getLocation().col);
        
    }
    else{
        current_entry = new SymbolEntry(p_function.getNameCString(), function, m_level,
                                    p_function.getTypeCString(), p_function.getArgumentCString());
        current_table->addSymbol(current_entry);
    }
    


    current_table = new SymbolTable;
    symbol_manager->pushScope(current_table);
    m_level++;
    flag_function = true;
    flag_parameter = true;
    in_function = true;

    p_function.visitChildNodes(*this);
    
    in_function = false;
    current_table = symbol_manager->topScope();
    if(opt_dump)
        current_table->dumpSymbol();
    symbol_manager->popScope();
    m_level--;
}

void SemanticAnalyzer::visit(CompoundStatementNode &p_compound_statement) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    bool later_pop = true;
    if (!flag_function)
    {
        current_table = new SymbolTable;
        symbol_manager->pushScope(current_table);
        m_level++;
    }
    else{
        flag_function = false;
        later_pop = false;
    }

    flag_parameter = false;

    p_compound_statement.visitChildNodes(*this);
    
    if( later_pop){
        current_table=symbol_manager->topScope();
        if(opt_dump)
            current_table->dumpSymbol();
        symbol_manager->popScope();
        m_level--;
    }
}

void SemanticAnalyzer::visit(PrintNode &p_print) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    p_print.visitChildNodes(*this);

    if(!p_print.checkInvalidChildren()){
        if( !p_print.checkTargetScalarType()){
            error_happen = true;
            std::cerr << "<Error> Found in line " << p_print.getLocation().line 
                    << ", column " << p_print.getTargetLocationCol()
                    << ": expression of print statement must be scalar type\n";
            std::cerr << "    " << source_code[p_print.getLocation().line] << '\n';
            showArrow(p_print.getTargetLocationCol());
        }
    }
}

void SemanticAnalyzer::visit(BinaryOperatorNode &p_bin_op) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    p_bin_op.visitChildNodes(*this);

    if(!p_bin_op.checkInvalidChildren()){

        const char *left_type = p_bin_op.getLeftTypeCString();
        const char *right_type = p_bin_op.getRightTypeCString();
        
        if(std::strcmp( p_bin_op.getOpCString(), "+")==0||std::strcmp( p_bin_op.getOpCString(), "-")==0||
        std::strcmp( p_bin_op.getOpCString(), "*")==0|| std::strcmp( p_bin_op.getOpCString(), "/")==0){

            if( std::strcmp( p_bin_op.getOpCString(), "+")==0 && 
                std::strcmp( left_type, "string")==0 && std::strcmp( right_type, "string")==0){
                p_bin_op.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kStringType));
            }
            else if(std::strcmp( left_type, "integer")==0 && std::strcmp( right_type, "integer")==0){
                p_bin_op.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kIntegerType));

                
            }
            else if(std::strcmp( left_type, "integer")==0 && std::strcmp( right_type, "real")==0){
                p_bin_op.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kRealType));
            }
            else if(std::strcmp( left_type, "real")==0 && std::strcmp( right_type, "real")==0){
                p_bin_op.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kRealType));
            }
            else if(std::strcmp( left_type, "real")==0 && std::strcmp( right_type, "integer")==0){
                p_bin_op.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kRealType));

            }
            else{
                error_happen = true;
                std::cerr << "<Error> Found in line " << p_bin_op.getLocation().line 
                        << ", column " << p_bin_op.getLocation().col << ": invalid operands to binary operator '" 
                        << p_bin_op.getOpCString() << "' ('" << p_bin_op.getLeftTypeCString() 
                        << "' and '" << p_bin_op.getRightTypeCString() <<  "')\n";
                std::cerr << "    " << source_code[p_bin_op.getLocation().line] << '\n';
                showArrow(p_bin_op.getLocation().col);
            }

        }
        else if(std::strcmp( p_bin_op.getOpCString(), "mod")==0){
            

            if(std::strcmp( left_type, "integer")==0 && std::strcmp( right_type, "integer")==0){
                p_bin_op.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kIntegerType));
            }
            else{
                error_happen = true;
                std::cerr << "<Error> Found in line " << p_bin_op.getLocation().line 
                        << ", column " << p_bin_op.getLocation().col << ": invalid operands to binary operator '" 
                        << p_bin_op.getOpCString() << "' ('" << p_bin_op.getLeftTypeCString() 
                        << "' and '" << p_bin_op.getRightTypeCString() <<  "')\n";
                std::cerr << "    " << source_code[p_bin_op.getLocation().line] << '\n';
                showArrow(p_bin_op.getLocation().col);
            }

        }
        else if(std::strcmp( p_bin_op.getOpCString(), "and")==0||std::strcmp( p_bin_op.getOpCString(), "or")==0){

            if(std::strcmp( left_type, "boolean")==0 && std::strcmp( right_type, "boolean")==0){
                p_bin_op.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kBoolType));
            }
            else{
                error_happen = true;
                std::cerr << "<Error> Found in line " << p_bin_op.getLocation().line 
                        << ", column " << p_bin_op.getLocation().col << ": invalid operands to binary operator '" 
                        << p_bin_op.getOpCString() << "' ('" << p_bin_op.getLeftTypeCString() 
                        << "' and '" << p_bin_op.getRightTypeCString() <<  "')\n";
                std::cerr << "    " << source_code[p_bin_op.getLocation().line] << '\n';
                showArrow(p_bin_op.getLocation().col);
            }
        }
        else{

            if(std::strcmp( left_type, "integer")==0 && std::strcmp( right_type, "integer")==0){
                p_bin_op.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kBoolType));

                
            }
            else if(std::strcmp( left_type, "integer")==0 && std::strcmp( right_type, "real")==0){
                p_bin_op.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kBoolType));
            }
            else if(std::strcmp( left_type, "real")==0 && std::strcmp( right_type, "real")==0){
                p_bin_op.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kBoolType));
            }
            else if(std::strcmp( left_type, "real")==0 && std::strcmp( right_type, "integer")==0){
                p_bin_op.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kBoolType));

            }
            else{
                error_happen = true;
                std::cerr << "<Error> Found in line " << p_bin_op.getLocation().line 
                        << ", column " << p_bin_op.getLocation().col << ": invalid operands to binary operator '" 
                        << p_bin_op.getOpCString() << "' ('" << p_bin_op.getLeftTypeCString() 
                        << "' and '" << p_bin_op.getRightTypeCString() <<  "')\n";
                std::cerr << "    " << source_code[p_bin_op.getLocation().line] << '\n';
                showArrow(p_bin_op.getLocation().col);
            }

        }

    }
    
}

void SemanticAnalyzer::visit(UnaryOperatorNode &p_un_op) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    p_un_op.visitChildNodes(*this);

    if(!p_un_op.checkInvalidChildren()){
        const char *type = p_un_op.getOperandTypeCString();
        
        if(std::strcmp( p_un_op.getOpCString(), "neg")==0){

            if(std::strcmp( type, "integer")==0){
                p_un_op.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kIntegerType));

                
            }
            else if(std::strcmp( type, "real")==0){
                p_un_op.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kRealType));
            }
            else{
                error_happen = true;
                std::cerr << "<Error> Found in line " << p_un_op.getLocation().line 
                        << ", column " << p_un_op.getLocation().col << ": invalid operand to unary operator '" 
                        << p_un_op.getOpCString() << "' ('" << p_un_op.getOperandTypeCString() << "')\n";
                std::cerr << "    " << source_code[p_un_op.getLocation().line] << '\n';
                showArrow(p_un_op.getLocation().col);
            }

        }
        else{
            if(std::strcmp( type, "boolean")==0){
                p_un_op.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kBoolType));
            }
            else{
                error_happen = true;
                std::cerr << "<Error> Found in line " << p_un_op.getLocation().line 
                        << ", column " << p_un_op.getLocation().col << ": invalid operand to unary operator '" 
                        << p_un_op.getOpCString() << "' ('" << p_un_op.getOperandTypeCString() << "')\n";
                std::cerr << "    " << source_code[p_un_op.getLocation().line] << '\n';
                showArrow(p_un_op.getLocation().col);
            }
        }
    }
}

void SemanticAnalyzer::visit(FunctionInvocationNode &p_func_invocation) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    p_func_invocation.visitChildNodes(*this);


    while( symbol_manager->getScopeSize()!=0){

        temp_current = symbol_manager->topScope();
        symbol_manager->popScope();
        temp_manager->pushScope(temp_current);

        if( temp_current->checkRedeclaration(p_func_invocation.getNameCString())){
            if(temp_current->getEntry(p_func_invocation.getNameCString())->getKindValue()!=function){
                
                
                error_happen = true;
                std::cerr << "<Error> Found in line " << p_func_invocation.getLocation().line 
                        << ", column " << p_func_invocation.getLocation().col << ": call of non-function symbol '" 
                        << p_func_invocation.getNameCString() << "'\n";
                std::cerr << "    " << source_code[p_func_invocation.getLocation().line] << '\n';
                showArrow(p_func_invocation.getLocation().col);
                break;
                
            }

            if( temp_current->getEntry(p_func_invocation.getNameCString())->getFunctionParameterNum()
            !=p_func_invocation.getArgumentsNum()){

                error_happen = true;
                std::cerr << "<Error> Found in line " << p_func_invocation.getLocation().line 
                        << ", column " << p_func_invocation.getLocation().col 
                        << ": too few/much arguments provided for function '" 
                        << p_func_invocation.getNameCString() << "'\n";
                std::cerr << "    " << source_code[p_func_invocation.getLocation().line] << '\n';
                showArrow(p_func_invocation.getLocation().col);

                break;
            }

            if(!p_func_invocation.checkInvalidChildren()){
                

                PTypeSharedPtr p_type;
                const SymbolEntry *this_entry = temp_current->getEntry(p_func_invocation.getNameCString());

                if( p_func_invocation.checkArgumentsType(this_entry->getAttributeCString())!=-1){
                    error_happen = true;
                    const int id = p_func_invocation.checkArgumentsType(this_entry->getAttributeCString());
                    std::cerr << "<Error> Found in line " << p_func_invocation.getLocation().line
                              << ", column " << p_func_invocation.getArgumentLocationCol(id)
                              << ": incompatible type passing '" << p_func_invocation.getArgumentTypeCString(id)
                              << "' to parameter of type '" 
                              << p_func_invocation.getParameterTypeCString(id, this_entry->getAttributeCString())
                              << "'\n";

                    std::cerr << "    " << source_code[p_func_invocation.getLocation().line] << '\n';
                    showArrow(p_func_invocation.getArgumentLocationCol(id));
                    break;
                }

                if( std::strcmp( this_entry->getTypeCString(), "void")==0)
                    p_func_invocation.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kVoidType));
                else if( std::strcmp( this_entry->getTypeCString(), "boolean")==0)
                    p_func_invocation.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kBoolType));
                else if( std::strcmp( this_entry->getTypeCString(), "string")==0)
                    p_func_invocation.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kStringType));
                else if( std::strcmp( this_entry->getTypeCString(), "integer")==0)
                    p_func_invocation.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kIntegerType));
                else if( std::strcmp( this_entry->getTypeCString(), "real")==0)
                    p_func_invocation.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kRealType));

            }

            break;
            
        }

        if( symbol_manager->getScopeSize()==0){
            error_happen = true;
            std::cerr << "<Error> Found in line " << p_func_invocation.getLocation().line 
                    << ", column " << p_func_invocation.getLocation().col << ": use of undeclared symbol '" 
                    << p_func_invocation.getNameCString() << "'\n";
     
            std::cerr << "    " << source_code[p_func_invocation.getLocation().line] << '\n';
            showArrow(p_func_invocation.getLocation().col);
            
        }
    }
    while( temp_manager->getScopeSize()!=0){
        symbol_manager->pushScope(temp_manager->topScope());
        temp_manager->popScope();
    }
    
}

void SemanticAnalyzer::visit(VariableReferenceNode &p_variable_ref) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */


    p_variable_ref.visitChildNodes(*this);

    if(!p_variable_ref.checkInvalidChildren()){
        if( !current_table->checkInErrorDeclaration(p_variable_ref.getNameCString())){


            while( symbol_manager->getScopeSize()!=0){

                temp_current = symbol_manager->topScope();
                symbol_manager->popScope();
                temp_manager->pushScope(temp_current);

                if( temp_current->checkRedeclaration(p_variable_ref.getNameCString())){
                    if(temp_current->getEntry(p_variable_ref.getNameCString())->getKindValue()==program ||
                temp_current->getEntry(p_variable_ref.getNameCString())->getKindValue()==function){
                        error_happen = true;
                    
                        std::cerr << "<Error> Found in line " << p_variable_ref.getLocation().line 
                                << ", column " << p_variable_ref.getLocation().col << ": use of non-variable symbol '" 
                                << p_variable_ref.getNameCString() << "'\n";

                        std::cerr << "    " << source_code[p_variable_ref.getLocation().line] << '\n';
                        showArrow(p_variable_ref.getLocation().col);
                        break;
                    }

                    if(p_variable_ref.getIndicesNum()
                        >temp_current->getEntry(p_variable_ref.getNameCString())->getTypeDimension()){
                        error_happen = true;
                        std::cerr << "<Error> Found in line " << p_variable_ref.getLocation().line
                                << ", column " << p_variable_ref.getLocation().col
                                << ": there is an over array subscript on '" 
                                << p_variable_ref.getNameCString() << "'\n";

                        std::cerr << "    " << source_code[p_variable_ref.getLocation().line] << '\n';
                        showArrow(p_variable_ref.getLocation().col);
                        break;
                    }

                                        if(p_variable_ref.checkNonIntegerIndices()!=-1){

                            error_happen = true;
                            std::cerr << "<Error> Found in line " << p_variable_ref.getLocation().line
                                    << ", column " << p_variable_ref.checkNonIntegerIndices()
                                    << ": index of array reference must be an integer\n";

                            std::cerr << "    " << source_code[p_variable_ref.getLocation().line] << '\n';
                            showArrow(p_variable_ref.checkNonIntegerIndices());
                            break;
                        }

                        // set type
                        PTypeSharedPtr p_type;
                        const SymbolEntry *this_entry = temp_current->getEntry(p_variable_ref.getNameCString());
                        const char *type_string = this_entry->getTypeCString();
                        const char *primitive_type;
                        if (std::strchr(type_string, '[') != NULL)
                        {
                            // array type
                            if(type_string[0]=='v') // void
                                primitive_type = "void";
                            else if(type_string[0]=='b') // boolean
                                primitive_type = "boolean";
                            else if(type_string[0]=='s') // string
                                primitive_type = "string";
                            else if(type_string[0]=='i') // integer
                                primitive_type = "integer";
                            else if(type_string[0]=='r') // real
                                primitive_type = "real";
                        }
                        else
                            primitive_type = type_string;

                        if( std::strcmp( primitive_type, "void")==0)
                            p_variable_ref.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kVoidType));
                        else if( std::strcmp( primitive_type, "boolean")==0)
                            p_variable_ref.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kBoolType));
                        else if( std::strcmp( primitive_type, "string")==0)
                            p_variable_ref.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kStringType));
                        else if( std::strcmp( primitive_type, "integer")==0)
                            p_variable_ref.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kIntegerType));
                        else if( std::strcmp( primitive_type, "real")==0)
                            p_variable_ref.setNodeType(std::make_shared<const PType>(PType::PrimitiveTypeEnum::kRealType));

                        std::vector<uint64_t> dims = this_entry->getNewTypeDimensions(p_variable_ref.getIndicesNum());
                        p_variable_ref.setNodeTypeDimensions(dims);

                        break;
                    
                    
                }

                if( symbol_manager->getScopeSize()==0){
                    error_happen = true;
                    std::cerr << "<Error> Found in line " << p_variable_ref.getLocation().line 
                            << ", column " << p_variable_ref.getLocation().col << ": use of undeclared symbol '" 
                            << p_variable_ref.getNameCString() << "'\n";
                    std::cerr << "    " << source_code[p_variable_ref.getLocation().line] << '\n';
                    showArrow(p_variable_ref.getLocation().col);
                }
            }
            while( temp_manager->getScopeSize()!=0){
                symbol_manager->pushScope(temp_manager->topScope());
                temp_manager->popScope();
            }
        }
    }
}

void SemanticAnalyzer::visit(AssignmentNode &p_assignment) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    p_assignment.visitChildNodes(*this);

    bool check = true;
    if (!p_assignment.checkInvalidLvalue())
    {

        if( !p_assignment.checkLvalueScalarType()){
            error_happen = true;
            std::cerr << "<Error> Found in line " << p_assignment.getLocation().line 
                    << ", column " << p_assignment.getLvalueLocationCol()
                    << ": array assignment is not allowed\n";
            std::cerr << "    " << source_code[p_assignment.getLocation().line] << '\n';
            showArrow(p_assignment.getLvalueLocationCol());
            check = false;
        }

        while( symbol_manager->getScopeSize()!=0){

            temp_current = symbol_manager->topScope();
            symbol_manager->popScope();
            temp_manager->pushScope(temp_current);

            if( temp_current->checkRedeclaration(p_assignment.getLvalueNameCString())){
                if(temp_current->getEntry(p_assignment.getLvalueNameCString())->getKindValue()==constant){
                    error_happen = true;
                
                    std::cerr << "<Error> Found in line " << p_assignment.getLocation().line 
                            << ", column " << p_assignment.getLvalueLocationCol()
                            << ": cannot assign to variable '" << p_assignment.getLvalueNameCString() 
                            << "' which is a constant\n";
                    std::cerr << "    " << source_code[p_assignment.getLocation().line] << '\n';
                    showArrow(p_assignment.getLvalueLocationCol());
                    check = false;
                    break;
                }

                 if(temp_current->getEntry(p_assignment.getLvalueNameCString())->getKindValue()==loop_var && !set_loop_var){
                    error_happen = true;
                
                    std::cerr << "<Error> Found in line " << p_assignment.getLocation().line 
                            << ", column " << p_assignment.getLvalueLocationCol()
                            << ": the value of loop variable cannot be modified inside the loop body\n";
                    std::cerr << "    " << source_code[p_assignment.getLocation().line] << '\n';
                    showArrow(p_assignment.getLvalueLocationCol());
                    check = false;
                    break;
                }
            }
                
        }
        while( temp_manager->getScopeSize()!=0){
            symbol_manager->pushScope(temp_manager->topScope());
            temp_manager->popScope();
        }


        if(!p_assignment.checkInvalidRvalue()){

            if( !p_assignment.checkRvalueScalarType() && check){
                error_happen = true;
                std::cerr << "<Error> Found in line " << p_assignment.getLocation().line 
                        << ", column " << p_assignment.getRvalueLocationCol()
                        << ": array assignment is not allowed\n";
                std::cerr << "    " << source_code[p_assignment.getLocation().line] << '\n';
                showArrow(p_assignment.getRvalueLocationCol());
                check = false;
            }

            if( !p_assignment.checkCompatibleLvalueRvalueType() && check){
                error_happen = true;
                std::cerr << "<Error> Found in line " << p_assignment.getLocation().line 
                        << ", column " << p_assignment.getLocation().col << ": assigning to '" 
                        << p_assignment.getLvalueTypeCString() << "' from incompatible type '"
                        << p_assignment.getRvalueTypeCString() << "'\n";
                std::cerr << "    " << source_code[p_assignment.getLocation().line] << '\n';
                showArrow(p_assignment.getLocation().col);
            }

            
        }
    }

    if(set_loop_var)
        set_loop_var = false;
}

void SemanticAnalyzer::visit(ReadNode &p_read) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    p_read.visitChildNodes(*this);

    if(!p_read.checkInvalidChildren()){

        if( !p_read.checkTargetScalarType()){
            error_happen = true;
            std::cerr << "<Error> Found in line " << p_read.getLocation().line 
                    << ", column " << p_read.getTargetLocationCol()
                    << ": variable reference of read statement must be scalar type\n";
            std::cerr << "    " << source_code[p_read.getLocation().line] << '\n';
            showArrow(p_read.getTargetLocationCol());
        }

        while( symbol_manager->getScopeSize()!=0){

            temp_current = symbol_manager->topScope();
            symbol_manager->popScope();
            temp_manager->pushScope(temp_current);

            if( temp_current->checkRedeclaration(p_read.getTargetNameCString())){
                if(temp_current->getEntry(p_read.getTargetNameCString())->getKindValue()==constant ||
            temp_current->getEntry(p_read.getTargetNameCString())->getKindValue()==loop_var){
                    error_happen = true;
                
                    std::cerr << "<Error> Found in line " << p_read.getLocation().line 
                            << ", column " << p_read.getTargetLocationCol()
                            << ": variable reference of read statement cannot be a constant or loop variable\n";
                    std::cerr << "    " << source_code[p_read.getLocation().line] << '\n';
                    showArrow(p_read.getTargetLocationCol());
                    break;
                }
            }
                
        }
        while( temp_manager->getScopeSize()!=0){
            symbol_manager->pushScope(temp_manager->topScope());
            temp_manager->popScope();
        }
    }
}

void SemanticAnalyzer::visit(IfNode &p_if) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    p_if.visitChildNodes(*this);

    if(!p_if.checkInvalidCondition()){

        if( !p_if.checkConditionBoolType()){
            error_happen = true;
            std::cerr << "<Error> Found in line " << p_if.getLocation().line 
                    << ", column " << p_if.getConditionLocationCol()
                    << ": the expression of condition must be boolean type\n";
            std::cerr << "    " << source_code[p_if.getLocation().line] << '\n';
            showArrow(p_if.getConditionLocationCol());
        }

    }

}

void SemanticAnalyzer::visit(WhileNode &p_while) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    p_while.visitChildNodes(*this);

    if(!p_while.checkInvalidCondition()){

        if( !p_while.checkConditionBoolType()){
            error_happen = true;
            std::cerr << "<Error> Found in line " << p_while.getLocation().line 
                    << ", column " << p_while.getConditionLocationCol()
                    << ": the expression of condition must be boolean type\n";
            std::cerr << "    " << source_code[p_while.getLocation().line] << '\n';
            showArrow(p_while.getConditionLocationCol());
        }

    }
}

void SemanticAnalyzer::visit(ForNode &p_for) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    // for loop variable
    current_table = new SymbolTable;
    symbol_manager->pushScope(current_table);
    m_level++;
    flag_for = true;
    set_loop_var = true;

    p_for.visitChildNodes(*this);
    
    if( atoi(p_for.getInitialValueCString())>atoi(p_for.getConditionValueCString())){
        error_happen = true;
        std::cerr << "<Error> Found in line " << p_for.getLocation().line 
                << ", column " << p_for.getLocation().col
                << ": the lower bound and upper bound of iteration count must be in the incremental order\n";
        std::cerr << "    " << source_code[p_for.getLocation().line] << '\n';
        showArrow(p_for.getLocation().col);
    }

    flag_for = false;
    set_loop_var = false;
    loop_variable.pop_back();
    current_table = symbol_manager->topScope();
    if(opt_dump)
        current_table->dumpSymbol();
    symbol_manager->popScope();
    m_level--;
}

void SemanticAnalyzer::visit(ReturnNode &p_return) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    p_return.visitChildNodes(*this);


    if(!in_function){
        error_happen = true;
        std::cerr << "<Error> Found in line " << p_return.getLocation().line 
                << ", column " << p_return.getLocation().col
                << ": program/procedure should not return a value\n";
        std::cerr << "    " << source_code[p_return.getLocation().line] << '\n';
        showArrow(p_return.getLocation().col);

    }
    else{
        
        // check whether in procedure
        while( symbol_manager->getScopeSize()!=0){

            temp_current = symbol_manager->topScope();
            symbol_manager->popScope();
            temp_manager->pushScope(temp_current);

            if( temp_current->checkFunctionInTable()!=-1){
                const SymbolEntry *function_entry = temp_current->getEntryWithId(temp_current->checkFunctionInTable());
                if (std::strcmp(function_entry->getTypeCString(), "void")==0)
                {
                    error_happen = true;
                    std::cerr << "<Error> Found in line " << p_return.getLocation().line 
                            << ", column " << p_return.getLocation().col
                            << ": program/procedure should not return a value\n";
                    std::cerr << "    " << source_code[p_return.getLocation().line] << '\n';
                    showArrow(p_return.getLocation().col);
                    break;
                }
                else{
                    if(!p_return.checkInvalidChildren()){
                        if(std::strcmp(function_entry->getTypeCString(), p_return.getReturnTypeCString())!=0){
                            error_happen = true;
                            std::cerr << "<Error> Found in line " << p_return.getLocation().line
                                      << ", column " << p_return.getReturnLocationCol()
                                      << ": return '" << p_return.getReturnTypeCString()
                                      << "' from a function with return type '" << function_entry->getTypeCString()
                                      << "'\n";
                            std::cerr << "    " << source_code[p_return.getLocation().line] << '\n';
                            showArrow(p_return.getReturnLocationCol());
                            
                        }

                        break;

                    }

                    break;
                }
            }
                
        }
        while( temp_manager->getScopeSize()!=0){
            symbol_manager->pushScope(temp_manager->topScope());
            temp_manager->popScope();
        }


    }

    
}


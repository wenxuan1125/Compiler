#include "codegen/CodeGenerator.hpp"
#include "visitor/AstNodeInclude.hpp"

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <stack>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <utility>

int local_addr = 8;
std::map<std::string, std::stack<int>> hash_addr;
std::stack<int> label_base;
int label_now;
bool main_function = true;
bool lvalue = false;
bool global_constant = false;
bool flag_if = false;
bool flag_branch = false;
bool flag_while = false;
bool flag_for = false;
bool flag_for_assign = false;

bool flag_invocation = false;
int parameter_id = 0;
int label = 1;

CodeGenerator::CodeGenerator(const char *in_file_name, const char *out_file_name, SymbolManager *symbol_manager) 
    : in_file_name(in_file_name), symbol_manager(symbol_manager) {
    this->out_fp = fopen(out_file_name, "w");
    assert(this->out_fp != NULL && "fopen() fails");
}

CodeGenerator::~CodeGenerator() {
    fclose(this->out_fp);
}

void CodeGenerator::dumpInstrs(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(this->out_fp, format, args);
    va_end(args);
}

void CodeGenerator::visit(ProgramNode &p_program) {
    // Reconstruct the hash table for looking up the symbol entry
    // Hint: Use symbol_manager->lookup(symbol_name) to get the symbol entry.
    symbol_manager->reconstructHashTableFromSymbolTable(p_program.getSymbolTable());
    local_addr = 8;

    // Generate RISC-V instructions for program header
    dumpInstrs("%s%s%s%s", 
        "   .file \"", this->in_file_name, "\"\n",
        "   .option nopic\n\n"
    );

    p_program.visitChildNodes(*this);


    dumpInstrs("%s%s%s%s%s%s", 
        "# in the function epilogue\n",
        "   lw ra, 124(sp)      # load return address saved in the current stack\n",
        "   lw s0, 120(sp)      # move frame pointer back to the bottom of the last stack\n",
        "   addi sp, sp, 128    # move stack pointer back to the top of the last stack\n",
        "   jr ra               # jump back to the caller function\n",
        "   .size main, .-main\n\n"
    );

    if(p_program.getSymbolTable()!=nullptr){
        const auto &entries = (p_program.getSymbolTable())->getEntries();
        for (const auto &entry : entries) {

            //dumpInstrs("*******%s\n", entry->getNameCString());
            //dumpInstrs("*******\n");

            if(hash_addr.find(entry->getName())!=hash_addr.end()){
                if(hash_addr[entry->getName()].size()==1){
                    hash_addr.erase(entry->getName());
                }
                else{
                    hash_addr[entry->getName()].pop();
                }
            }
            
        }

    }
    

    // Remove the entries in the hash table
    symbol_manager->removeSymbolsFromHashTable(p_program.getSymbolTable());
}

void CodeGenerator::visit(DeclNode &p_decl) {
    p_decl.visitChildNodes(*this);
}

void CodeGenerator::visit(VariableNode &p_variable) {
    const SymbolEntry *entry = symbol_manager->lookup(p_variable.getName());
    if(entry!=nullptr){

        // global variable
        if(entry->getLevel()==0 && entry->getKind()==SymbolEntry::KindEnum::kVariableKind){

            dumpInstrs("# global variable declaration: %s\n", p_variable.getNameCString());
            dumpInstrs("%s%s%s",
                ".comm ",
                p_variable.getNameCString(),
                ", 4, 4\n\n" 
            );
        }
        // global constant
        else if(entry->getLevel()==0 && entry->getKind()==SymbolEntry::KindEnum::kConstantKind){

            dumpInstrs("# global constant declaration: %s\n", p_variable.getNameCString());
            dumpInstrs("%s%s%s%s%s%s%s%s",
                ".section    .rodata\n", 
                "   .align 2\n",
                "   .globl ",
                p_variable.getNameCString(),
                "\n",
                "   .type ",
                p_variable.getNameCString(),
                ", @object\n"
            );

            dumpInstrs("%s%s%s",
                p_variable.getNameCString(),
                ":\n",
                "    .word "
            );

            global_constant = true;
            p_variable.visitChildNodes(*this);
            dumpInstrs("\n\n");
        }
        // local variable, loop variable
        else if(entry->getKind()==SymbolEntry::KindEnum::kVariableKind
            || entry->getKind()==SymbolEntry::KindEnum::kLoopVarKind){
            

            if(hash_addr.find(entry->getName())!=hash_addr.end()){
                hash_addr[entry->getName()].push(local_addr);
            }
            else{
                std::stack<int> addr_stack;
                addr_stack.push(local_addr);
                hash_addr[entry->getName()] = addr_stack;
            }

            //dumpInstrs("********\n");
            //dumpInstrs("%s%s%d\n", p_variable.getNameCString(), ": ", local_addr);

            if(p_variable.getTypePtr()->isScalar()){
                local_addr += 4;
            }
            else{
                int element_num = 1;

                for (auto dimension : p_variable.getTypePtr()->getDimensions()) {
                    element_num *= dimension;
                }

                local_addr += (4 * element_num);
            }
        }
        // local constant
        else if(entry->getKind()==SymbolEntry::KindEnum::kConstantKind){

            
            if(hash_addr.find(entry->getName())!=hash_addr.end()){
                hash_addr[entry->getName()].push(local_addr);
            }
            else{
                std::stack<int> addr_stack;
                addr_stack.push(local_addr);
                hash_addr[entry->getName()] = addr_stack;
            }

            dumpInstrs("# local constant declaration: %s\n", p_variable.getNameCString());
            dumpInstrs("%s%d%s%s%s",
                "   addi t0, s0, -",
                local_addr + 4,
                "\n",
                "   addi sp, sp, -4\n",
                "   sw t0, 0(sp)        # push the address to the stack\n"
            );
            local_addr += 4;

            p_variable.visitChildNodes(*this);


            dumpInstrs("%s%s%s%s%s",

                "   lw t0, 0(sp)        # pop the value from the stack\n",
                "   addi sp, sp, 4\n",
                "   lw t1, 0(sp)        # pop the address from the stack\n",
                "   addi sp, sp, 4\n",
                "   sw t0, 0(t1)\n"
            );

            
            
        }

        // function parameter
        else if(entry->getKind()==SymbolEntry::KindEnum::kParameterKind){
            

            if(hash_addr.find(entry->getName())!=hash_addr.end()){
                hash_addr[entry->getName()].push(local_addr);
            }
            else{
                std::stack<int> addr_stack;
                addr_stack.push(local_addr);
                hash_addr[entry->getName()] = addr_stack;
            }

            //dumpInstrs("********\n");
            if(parameter_id<=7){

                if(p_variable.getTypePtr()->isScalar()){

                    dumpInstrs("%s%d%s%d%s%s%s", 
                        "   sw a", 
                        parameter_id,
                        ", -",
                        local_addr + 4,
                        "(s0)      # save parameter ",
                        p_variable.getNameCString(),
                        " in the local stack\n"
                    );

                }
                else{
                    
                    int element_num = 1;

                    for (auto dimension : p_variable.getTypePtr()->getDimensions()) {
                        element_num *= dimension;
                    }

                    int addr = hash_addr[entry->getName()].top();
                    
                    for( int i=0; i<element_num; i++){

                        if( i<=7){
                            dumpInstrs("%s%d%s%d%s%s%s", 
                                "   sw a", 
                                i,
                                ", -",
                                addr + 4,
                                "(s0)      # save parameter ",
                                p_variable.getNameCString(),
                                " in the local stack\n"
                            );

                        }
                        else{

                            dumpInstrs("%s%d%s%d%s%s%s", 
                                "   sw s", 
                                i - 7,
                                ", -",
                                addr + 4,
                                "(s0)      # save parameter ",
                                p_variable.getNameCString(),
                                " in the local stack\n"
                            );

                        }

                        addr += 4;

                    }

                }
                
            }
            else{

                dumpInstrs("%s%d%s%d%s%s%s", 
                    "   sw s", 
                    parameter_id - 7,
                    ", -",
                    local_addr + 4,
                    "(s0)      # save parameter ",
                    p_variable.getNameCString(),
                    " in the local stack\n"
                );

            }

            parameter_id++;

            if(p_variable.getTypePtr()->isScalar()){
                local_addr += 4;
            }
            else{
                int element_num = 1;

                for (auto dimension : p_variable.getTypePtr()->getDimensions()) {
                    element_num *= dimension;
                }

                local_addr += (4 * element_num);
            }
        }
        
    }
    //p_variable.visitChildNodes(*this);
}

void CodeGenerator::visit(ConstantValueNode &p_constant_value) {

    if(global_constant){
        global_constant = false;
        dumpInstrs("%s",
            p_constant_value.getConstantValueCString()
        );
    }
    else{

        if(p_constant_value.getTypePtr()->isInteger()){
            dumpInstrs("%s%s%s%s%s", 
                "   li t0, ",
                p_constant_value.getConstantValueCString(),
                "            # load value to register 't0'\n",
                "   addi sp, sp, -4\n",
                "   sw t0, 0(sp)        # push the value to the stack\n"
            );
        }
        else if(p_constant_value.getTypePtr()->isBool()){
            dumpInstrs("%s", "   li t0, ");

            if(std::strcmp(p_constant_value.getConstantValueCString(), "true")==0)
                dumpInstrs("%d", 1);
            else
                dumpInstrs("%d", 0);

            dumpInstrs("%s%s%s",     
                "            # load value to register 't0'\n",
                "   addi sp, sp, -4\n",
                "   sw t0, 0(sp)        # push the value to the stack\n"
            );
        }

        
    }

    //dumpInstrs("%%%%%%%%%\n");
}

void CodeGenerator::visit(FunctionNode &p_function) {
    // Reconstruct the hash table for looking up the symbol entry
    symbol_manager->reconstructHashTableFromSymbolTable(p_function.getSymbolTable());
    local_addr = 8;

    main_function = false;

    dumpInstrs("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", 
        ".section    .text\n",
        "   .align 2\n",
        "   .globl ",
        p_function.getNameCString(),
        "\n",
        "   .type ",
        p_function.getNameCString(),
        ", @function\n\n",
        p_function.getNameCString(),
        
        ":\n",
        "# in the function prologue\n",
        "   addi sp, sp, -128   # move stack pointer to lower address to allocate a new stack\n",
        "   sw ra, 124(sp)      # save return address of the caller function in the current stack\n",
        "   sw s0, 120(sp)      # save frame pointer of the last stack in the current stack\n",
        "   addi s0, sp, 128    # move frame pointer to the bottom of the current stack\n\n"
    );

    //parameter_num = p_function.getParametersNum(p_function.getParameters());

    p_function.visitChildNodes(*this);

    parameter_id = 0;
    main_function = true;
    local_addr = 8;


    dumpInstrs("%s%s%s%s%s%s%s%s%s%s", 
        "# in the function epilogue\n",
        "   lw ra, 124(sp)      # load return address saved in the current stack\n",
        "   lw s0, 120(sp)      # move frame pointer back to the bottom of the last stack\n",
        "   addi sp, sp, 128    # move stack pointer back to the top of the last stack\n",
        "   jr ra               # jump back to the caller function\n",
        "   .size ",
        p_function.getNameCString(),
        ", .-",
        p_function.getNameCString(),
        "\n\n"
    );

    

    if(p_function.getSymbolTable()!=nullptr){
        const auto &entries = (p_function.getSymbolTable())->getEntries();
        for (const auto &entry : entries) {

            //dumpInstrs("*******%s\n", entry->getNameCString());
            //dumpInstrs("*******\n");

            if(hash_addr.find(entry->getName())!=hash_addr.end()){
                if(hash_addr[entry->getName()].size()==1){
                    hash_addr.erase(entry->getName());
                }
                else{
                    hash_addr[entry->getName()].pop();
                }
            }
            
        }

    }
    

    // Remove the entries in the hash table
    symbol_manager->removeSymbolsFromHashTable(p_function.getSymbolTable());
}

void CodeGenerator::visit(CompoundStatementNode &p_compound_statement) {
    // Reconstruct the hash table for looking up the symbol entry
    symbol_manager->reconstructHashTableFromSymbolTable(p_compound_statement.getSymbolTable());

    

    if(main_function){
        dumpInstrs("%s%s%s%s%s%s%s%s%s%s", 
            ".section    .text\n",
            "   .align 2\n",
            "   .globl main         # emit symbol 'main' to the global symbol table\n",
            "   .type main, @function\n\n",

            
            "main:\n",
            "# in the function prologue\n",
            "   addi sp, sp, -128   # move stack pointer to lower address to allocate a new stack\n",
            "   sw ra, 124(sp)      # save return address of the caller function in the current stack\n",
            "   sw s0, 120(sp)      # save frame pointer of the last stack in the current stack\n",
            "   addi s0, sp, 128    # move frame pointer to the bottom of the current stack\n\n"
        );
    }

    
    if (flag_if)
    {
        
        dumpInstrs("L%d:\n", label_now);
    }

    if(flag_while){
        dumpInstrs("L%d:\n", label_now + 1);
    }

    if(flag_for){


        dumpInstrs("%s%s%s%s%s%d%s", 
            "   lw t0, 0(sp)        # pop the value from the stack\n",
            "   addi sp, sp, 4\n",
            "   lw t1, 0(sp)        # pop the value from the stack\n",
            "   addi sp, sp, 4\n",
            "   bge t1, t0, L",
            label_now + 2,
            "        # if i >= condition value, exit the loop\n"
        );

        dumpInstrs("L%d:\n", label_now + 1);

    }

    p_compound_statement.visitChildNodes(*this);

    if(flag_if){
        dumpInstrs("   j L%d                # jump to L%d\n", label_now + 2, label_now + 2);
        dumpInstrs("L%d:\n", label_now + 1);
        flag_if = false;
    }
    if(flag_while){
        dumpInstrs("   j L%d                # jump to L%d\n", label_now, label_now);
        flag_while = false;
    }

    if(p_compound_statement.getSymbolTable()!=nullptr){
        const auto &entries = (p_compound_statement.getSymbolTable())->getEntries();
        for (const auto &entry : entries) {

            if(hash_addr.find(entry->getName())!=hash_addr.end()){
                if(hash_addr[entry->getName()].size()==1){
                    hash_addr.erase(entry->getName());
                }
                else{
                    hash_addr[entry->getName()].pop();
                }
            }

        }
    }
     

     // Remove the entries in the hash table
     symbol_manager->removeSymbolsFromHashTable(p_compound_statement.getSymbolTable());
}

void CodeGenerator::visit(PrintNode &p_print) {

    dumpInstrs("\n");
    dumpInstrs("# print\n");


    p_print.visitChildNodes(*this);


    dumpInstrs("%s%s%s", 
        "   lw a0, 0(sp)        # pop the value from the stack to the first argument register 'a0'\n",
        "   addi sp, sp, 4\n",
        "   jal ra, printInt    # call function 'printInt'\n\n"
    );
    
}

void CodeGenerator::visit(BinaryOperatorNode &p_bin_op) {

    dumpInstrs("\n");
    dumpInstrs("# binary operator: %s\n", p_bin_op.getOpCString());

    bool branch = false;
    if(flag_branch){
        flag_branch = false;
        branch = true;
    }

    p_bin_op.visitChildNodes(*this);


    dumpInstrs("%s%s%s%s", 
        "   lw t0, 0(sp)        # pop the value from the stack\n",
        "   addi sp, sp, 4\n",
        "   lw t1, 0(sp)        # pop the value from the stack\n",
        "   addi sp, sp, 4\n"
    );

    if (std::strcmp(p_bin_op.getOpCString(), "+")==0){
        dumpInstrs("   add t0, t1, t0      # always save the value in a certain register you choose\n");

        dumpInstrs("%s%s", 
            "   addi sp, sp, -4\n",
            "   sw t0, 0(sp)        # push the value to the stack\n"
        );
    }
    else if (std::strcmp(p_bin_op.getOpCString(), "-")==0){
        dumpInstrs("   sub t0, t1, t0      # always save the value in a certain register you choose\n");

        dumpInstrs("%s%s", 
            "   addi sp, sp, -4\n",
            "   sw t0, 0(sp)        # push the value to the stack\n"
        );
    }
    
    else if (std::strcmp(p_bin_op.getOpCString(), "*")==0){
        dumpInstrs("   mul t0, t1, t0      # always save the value in a certain register you choose\n");

        dumpInstrs("%s%s", 
            "   addi sp, sp, -4\n",
            "   sw t0, 0(sp)        # push the value to the stack\n"
        );
    }
    else if (std::strcmp(p_bin_op.getOpCString(), "/")==0){
        dumpInstrs("   div t0, t1, t0      # always save the value in a certain register you choose\n");

        dumpInstrs("%s%s", 
            "   addi sp, sp, -4\n",
            "   sw t0, 0(sp)        # push the value to the stack\n"
        );
    }
    else if (std::strcmp(p_bin_op.getOpCString(), "mod")==0){
        dumpInstrs("   rem t0, t1, t0      # always save the value in a certain register you choose\n");

        dumpInstrs("%s%s", 
            "   addi sp, sp, -4\n",
            "   sw t0, 0(sp)        # push the value to the stack\n"
        );
    }

    else if (std::strcmp(p_bin_op.getOpCString(), "and")==0){
        dumpInstrs("   and t0, t1, t0      # always save the value in a certain register you choose\n");

        dumpInstrs("%s%s", 
            "   addi sp, sp, -4\n",
            "   sw t0, 0(sp)        # push the value to the stack\n"
        );
    }
    else if (std::strcmp(p_bin_op.getOpCString(), "or")==0){
        dumpInstrs("   or t0, t1, t0      # always save the value in a certain register you choose\n");

        dumpInstrs("%s%s", 
            "   addi sp, sp, -4\n",
            "   sw t0, 0(sp)        # push the value to the stack\n"
        );
    }

    else if (std::strcmp(p_bin_op.getOpCString(), "<=")==0){

        if(flag_if && branch){


            dumpInstrs("%s%d%s%d%s",
                "   bgt t1, t0, L",
                label_now + 1,
                "      # if t1 > t0, jump to L",
                label_now + 1,
                "\n"
            );
        }
        else if(flag_while && branch){
            dumpInstrs("%s%d%s%d%s",
                "   bgt t1, t0, L",
                label_now + 2,
                "      # if t1 > t0, jump to L",
                label_now + 2,
                "\n"
            );
        }
        else{

            dumpInstrs("   sub t0, t1, t0      # always save the value in a certain register you choose\n");
            dumpInstrs("   slti t0, t0, 1\n");


            dumpInstrs("%s%s", 
                "   addi sp, sp, -4\n",
                "   sw t0, 0(sp)        # push the value to the stack\n"
            );

        }
        
        
    }
    else if (std::strcmp(p_bin_op.getOpCString(), "<")==0){

        if(flag_if && branch){

            dumpInstrs("%s%d%s%d%s",
                "   bge t1, t0, L",
                label_now + 1,
                "      # if t1 > t0, jump to L",
                label_now + 1,
                "\n"
            );
        }
        else if(flag_while && branch){
            dumpInstrs("%s%d%s%d%s",
                "   bge t1, t0, L",
                label_now + 2,
                "      # if t1 > t0, jump to L",
                label_now + 2,
                "\n"
            );
        }
        else{

            dumpInstrs("   sub t0, t1, t0      # always save the value in a certain register you choose\n");
            dumpInstrs("   slti t0, t0, 0\n");


            dumpInstrs("%s%s", 
                "   addi sp, sp, -4\n",
                "   sw t0, 0(sp)        # push the value to the stack\n"
            );
            
        }

        

    }
    else if (std::strcmp(p_bin_op.getOpCString(), ">=")==0){

        if(flag_if && branch){

            dumpInstrs("%s%d%s%d%s",
                "   blt t1, t0, L",
                label_now + 1,
                "      # if t1 > t0, jump to L",
                label_now + 1,
                "\n"
            );
        }
        else if(flag_while && branch){
            dumpInstrs("%s%d%s%d%s",
                "   blt t1, t0, L",
                label_now + 2,
                "      # if t1 > t0, jump to L",
                label_now + 2,
                "\n"
            );
        }
        else{

            dumpInstrs("   sub t0, t1, t0      # always save the value in a certain register you choose\n");
            dumpInstrs("   slti t0, t0, 0\n");
            dumpInstrs("   slti t0, t0, 1\n");
            //dumpInstrs("   xori t0, t0, -1\n");


            dumpInstrs("%s%s", 
                "   addi sp, sp, -4\n",
                "   sw t0, 0(sp)        # push the value to the stack\n"
            );
            
        }


    }
    else if (std::strcmp(p_bin_op.getOpCString(), ">")==0){


        if (flag_if && branch)
        {

            dumpInstrs("%s%d%s%d%s",
                "   ble t1, t0, L",
                label_now + 1,
                "      # if t1 > t0, jump to L",
                label_now + 1,
                "\n"
            );
        }
        else if(flag_while && branch){
            dumpInstrs("%s%d%s%d%s",
                "   ble t1, t0, L",
                label_now + 2,
                "      # if t1 > t0, jump to L",
                label_now + 2,
                "\n"
            );
        }
        else{

            dumpInstrs("   sub t0, t1, t0      # always save the value in a certain register you choose\n");
            dumpInstrs("   slti t0, t0, 1\n");
            dumpInstrs("   slti t0, t0, 1\n");
            //dumpInstrs("   xori t0, t0, -1\n");


            dumpInstrs("%s%s", 
                "   addi sp, sp, -4\n",
                "   sw t0, 0(sp)        # push the value to the stack\n"
            );
            
        }


    }
    else if (std::strcmp(p_bin_op.getOpCString(), "=")==0){

        if(flag_if && branch){

            dumpInstrs("%s%d%s%d%s",
                "   bne t1, t0, L",
                label_now + 1,
                "      # if t1 > t0, jump to L",
                label_now + 1,
                "\n"
            );
        }
        else if(flag_while && branch){
            dumpInstrs("%s%d%s%d%s",
                "   bne t1, t0, L",
                label_now + 2,
                "      # if t1 > t0, jump to L",
                label_now + 2,
                "\n"
            );
        }
        else{

            // dumpInstrs("   sub t0, t1, t0      # always save the value in a certain register you choose\n");
            // dumpInstrs("   slti t0, t0, 1\n");
            // dumpInstrs("   xori t0, t0, -1\n");


            // dumpInstrs("%s%s", 
            //     "   addi sp, sp, -4\n",
            //     "   sw t0, 0(sp)        # push the value to the stack\n"
            // );
            
        }

    }
    else if (std::strcmp(p_bin_op.getOpCString(), "<>")==0){

        if(flag_if && branch){
            dumpInstrs("%s%d%s%d%s",
                "   beq t1, t0, L",
                label_now + 1,
                "      # if t1 > t0, jump to L",
                label_now + 1,
                "\n"
            );
        }
        else if(flag_while && branch){
            dumpInstrs("%s%d%s%d%s",
                "   beq t1, t0, L",
                label_now + 2,
                "      # if t1 > t0, jump to L",
                label_now + 2,
                "\n"
            );
        }
        else{
            
        }
      
    }

    

    
    dumpInstrs("\n");
}

void CodeGenerator::visit(UnaryOperatorNode &p_un_op) {


    if (std::strcmp(p_un_op.getOpCString(), "neg")==0){

        dumpInstrs("\n");
        dumpInstrs("# unary operator: %s\n", p_un_op.getOpCString());

        dumpInstrs("%s%s%s", 
            "   li t0, -1\n",
            "   addi sp, sp, -4\n",
            "   sw t0, 0(sp)        # push the value to the stack\n"
        );


        p_un_op.visitChildNodes(*this);
        

        dumpInstrs("%s%s%s%s", 
            "   lw t0, 0(sp)        # pop the value from the stack\n",
            "   addi sp, sp, 4\n",
            "   lw t1, 0(sp)        # pop the value from the stack\n",
            "   addi sp, sp, 4\n"
        );


        dumpInstrs("   mul t0, t1, t0      # always save the value in a certain register you choose\n");
        dumpInstrs("%s%s", 
            "   addi sp, sp, -4\n",
            "   sw t0, 0(sp)        # push the value to the stack\n"
        );
        dumpInstrs("\n");

    }

    else if (std::strcmp(p_un_op.getOpCString(), "not")==0){

        dumpInstrs("\n");
        dumpInstrs("# unary operator: %s\n", p_un_op.getOpCString());

        bool branch = false;
        if (flag_branch){
            flag_branch = false;
            branch = true;
        }

        p_un_op.visitChildNodes(*this);
    

        dumpInstrs("%s%s", 
            "   lw t0, 0(sp)        # pop the value from the stack\n",
            "   addi sp, sp, 4\n"
        );

        dumpInstrs("   slti t0, t0, 1\n");
        dumpInstrs("%s%s", 
            "   addi sp, sp, -4\n",
            "   sw t0, 0(sp)        # push the value to the stack\n"
        );
        dumpInstrs("\n");



        if(branch){
             dumpInstrs("%s%s%s", 
                "   lw t1, 0(sp)        # pop the value from the stack\n",
                "   addi sp, sp, 4\n",
                "   li t0, 0\n"
            );
            

            dumpInstrs("%s%d%s%d%s",
                "   beq t1, t0, L",
                label_now + 1,
                "      # if t1 == 0, jump to L",
                label_now + 1,
                "\n"
            );

        }
        

    }



    
}

void CodeGenerator::visit(FunctionInvocationNode &p_func_invocation) {

    dumpInstrs("\n%s\n",
        "# function invocation: ",
        p_func_invocation.getNameCString()
    );

    flag_invocation = true;
    p_func_invocation.visitChildNodes(*this);

    flag_invocation = false;

    for (int i = p_func_invocation.getArguments().size()-1; i >= 0; i--){
        if( i<=7){

            if(p_func_invocation.getArguments()[i]->getInferredType()->isScalar()){

                dumpInstrs("%s%d%s%d%s%s", 
                    "   lw a",
                    i,
                    ", 0(sp)        # pop the value from the stack to the argument register a",
                    i,
                    "\n",
                    "   addi sp, sp, 4\n"
                );

                

            }
            else{

                int element_num = 1;

                for (auto dimension : p_func_invocation.getArguments()[i]->getInferredType()->getDimensions()) {
                    element_num *= dimension;
                }

                int register_id;
                if(element_num>8)
                    register_id = 7;
                else
                    register_id = element_num - 1;


                for (int j = 0; j < element_num; j++)
                {
                    if(j<element_num-8){
                        // use saved register from s1
                        dumpInstrs("%s%d%s%d%s%s", 
                            "   lw s",
                            element_num - 8 - j,
                            ", 0(sp)        # pop the value from the stack to the argument register s",
                            element_num - 8 - j,
                            "\n",
                            "   addi sp, sp, 4\n"
                        );

                    }
                    else{

                        dumpInstrs("%s%d%s%d%s%s", 
                            "   lw a",
                            register_id,
                            ", 0(sp)        # pop the value from the stack to the argument register a",
                            register_id,
                            "\n",
                            "   addi sp, sp, 4\n"
                        );

                        register_id--;
                    }
                }
            }

    
            
        }
        else{

            dumpInstrs("%s%d%s%d%s%s", 
                "   lw s",
                i - 7,
                ", 0(sp)        # pop the value from the stack to the argument register s",
                i - 7,
                "\n",
                "   addi sp, sp, 4\n"
            );

            

        }
        
    }


    dumpInstrs("%s%s%s%s%s%s%s%s", 
        "   jal ra, ",
        p_func_invocation.getNameCString(),
        "         # call function ",
        p_func_invocation.getNameCString(),
        "\n",
        "   mv t0, a0           # always move the return value to a certain register you choose\n",
        "   addi sp, sp, -4\n",
        "   sw t0, 0(sp)        # push the value to the stack\n\n"
    );

    dumpInstrs("\n");

}

void CodeGenerator::visit(VariableReferenceNode &p_variable_ref) {
    const SymbolEntry *entry = symbol_manager->lookup(p_variable_ref.getName());
    if(entry!=nullptr){
        // global variable
        if(entry->getLevel()==0 && entry->getKind()==SymbolEntry::KindEnum::kVariableKind){

            if(lvalue){
                lvalue = false;
                dumpInstrs("%s%s%s%s%s%s%s", 
                    "   la t0, ",
                    p_variable_ref.getNameCString(),
                    "           # load the address of variable ",
                    p_variable_ref.getNameCString(),
                    "\n",
                    "   addi sp, sp, -4\n",
                    "   sw t0, 0(sp)     # push the address to the stack\n"
                );

            }
            else{

                dumpInstrs("%s%s%s%s%s%s%s%s%s", 
                    "   la t0, ",
                    p_variable_ref.getNameCString(),
                    "\n",
                    "   lw t1, 0(t0)        # load the value of ",
                    p_variable_ref.getNameCString(),
                    "\n",
                    "   mv t0, t1\n",
                    "   addi sp, sp, -4\n",
                    "   sw t0, 0(sp)        # push the value to the stack\n"
                );

            }
            
        }
        // global constant
        else if(entry->getLevel()==0 && entry->getKind()==SymbolEntry::KindEnum::kConstantKind){

            if(!lvalue){
                dumpInstrs("%s%s%s%s%s%s%s%s%s", 
                    "   la t0, ",
                    p_variable_ref.getNameCString(),
                    "\n",
                    "   lw t1, 0(t0)        # load the value of ",
                    p_variable_ref.getNameCString(),
                    "\n",
                    "   mv t0, t1\n",
                    "   addi sp, sp, -4\n",
                    "   sw t0, 0(sp)        # push the value to the stack\n"
                );
            }
            

        }
        // local variable, function parameter
        else if(entry->getLevel()!=0 
            && (entry->getKind()==SymbolEntry::KindEnum::kVariableKind 
            || entry->getKind()==SymbolEntry::KindEnum::kParameterKind
            || entry->getKind()==SymbolEntry::KindEnum::kLoopVarKind)){

            int addr;
            addr=hash_addr[entry->getName()].top();

            if(lvalue){
                lvalue = false;

                if(entry->getTypePtr()->isScalar()){
                    dumpInstrs("%s%d%s%s%s",
                        "   addi t0, s0, -",
                        addr + 4,
                        "\n",
                        "   addi sp, sp, -4\n",
                        "   sw t0, 0(sp)        # push the address to the stack\n"
                    );
                }
                else{

                    dumpInstrs("# array reference lvalue\n");

                    p_variable_ref.visitChildNodes(*this);

                    int times = 1;

                    dumpInstrs("\n# count offset---------------\n");
                    dumpInstrs("   li t2, 0\n");

                    for (int i = p_variable_ref.getIndices().size() - 1; i >= 0; i--)
                    {

                        if(i != p_variable_ref.getIndices().size() - 1)
                            times *= entry->getTypePtr()->getDimensions()[i + 1];

                        dumpInstrs("%s%s%s",
                            "   lw t0, 0(sp)        # pop the value from the stack\n",
                            "   addi t0, t0, -1     # index starts from 1\n",
                            "   addi sp, sp, 4\n"
                        );

                        dumpInstrs("   li t1, %d\n", times);

                        dumpInstrs("%s%s",
                            "   mul t1, t0, t1\n",
                            "   add t2, t1, t2\n"
                        );
                    }


                    dumpInstrs("   li t1, 4\n");
                    dumpInstrs("   mul t2, t2, t1\n");
                    dumpInstrs("   addi t2, t2, %d\n", addr);
                    dumpInstrs("   addi t2, t2, 4\n");
                    dumpInstrs("# count offset end-----------\n\n");


                    dumpInstrs("%s%s%s",
                        "   sub t0, s0, t2\n",
                        "   addi sp, sp, -4\n",
                        "   sw t0, 0(sp)        # push the address to the stack\n"
                    );


                }

            }
            else{

                if(entry->getTypePtr()->isScalar()){
                    dumpInstrs("%s%d%s%s%s%s%s",
                        "   lw t0, -",
                        addr + 4,
                        "(s0)      # load the value of ",
                        p_variable_ref.getNameCString(),
                        "\n",
                        "   addi sp, sp, -4\n",
                        "   sw t0, 0(sp)        # push the value to the stack\n"
                    );
                }
                else{


                    if(flag_invocation){

                        dumpInstrs("# array reference rvalue\n");

                        p_variable_ref.visitChildNodes(*this);

                        int element_num = 1;

                        for (auto dimension : p_variable_ref.getInferredType()->getDimensions()) {
                            element_num *= dimension;
                        }



                        int addr;
                        addr=hash_addr[entry->getName()].top();

                        for( int i=0; i<element_num; i++){

                            dumpInstrs("%s%d%s%s%s%s%s",
                                "   lw t0, -",
                                addr + 4,
                                "(s0)      # load the value of ",
                                p_variable_ref.getNameCString(),
                                "\n",
                                "   addi sp, sp, -4\n",
                                "   sw t0, 0(sp)        # push the value to the stack\n"
                            );

                            addr += 4;

                        }

                        

                    }
                    else{

                        dumpInstrs("# array reference rvalue\n");

                        p_variable_ref.visitChildNodes(*this);

                        int times = 1;

                        dumpInstrs("\n# count offset---------------\n");
                        dumpInstrs("   li t2, 0\n");

                        for (int i = p_variable_ref.getIndices().size() - 1; i >= 0; i--)
                        {

                            if(i != p_variable_ref.getIndices().size() - 1)
                                times *= entry->getTypePtr()->getDimensions()[i + 1];

                            dumpInstrs("%s%s%s",
                                "   lw t0, 0(sp)        # pop the value from the stack\n",
                                "   addi t0, t0, -1     # index starts from 1\n",
                                "   addi sp, sp, 4\n"
                            );

                            dumpInstrs("   li t1, %d\n", times);

                            dumpInstrs("%s%s",
                                "   mul t1, t0, t1\n",
                                "   add t2, t1, t2\n"
                            );
                        }

                        dumpInstrs("   li t1, 4\n");
                        dumpInstrs("   mul t2, t2, t1\n");
                        dumpInstrs("   addi t2, t2, %d\n", addr);
                        dumpInstrs("   addi t2, t2, 4\n");
                        dumpInstrs("# count offset end-----------\n\n");

                        dumpInstrs("%s%s%s%s",
                            "   sub t0, s0, t2\n",
                            "   lw t0, 0(t0)\n",
                            "   addi sp, sp, -4\n",
                            "   sw t0, 0(sp)        # push the value to the stack\n"
                        );


                    }
                    
                    
                }

                
            }

        }
        // local constant
        else if(entry->getLevel()!=0 && entry->getKind()==SymbolEntry::KindEnum::kConstantKind){

            int addr;
            
            addr=hash_addr[entry->getName()].top();

            if(!lvalue){
                dumpInstrs("%s%d%s%s%s%s%s",
                    "   lw t0, -",
                    addr + 4,
                    "(s0)       # load the value of ",
                    p_variable_ref.getNameCString(),
                    "\n",
                    "   addi sp, sp, -4\n",
                    "   sw t0, 0(sp)        # push the value to the stack\n"
                );
            }
            

        }


        if(flag_branch){

            flag_branch = false;

            dumpInstrs("%s%s%s", 
                "   lw t1, 0(sp)        # pop the value from the stack\n",
                "   addi sp, sp, 4\n",
                "   li t0, 0\n"
            );
            
            dumpInstrs("%s%d%s%d%s",
                "   beq t1, t0, L",
                label_now + 1,
                "      # if t1 == 0, jump to L",
                label_now + 1,
                "\n"
            );
        }
    }
    //p_variable_ref.visitChildNodes(*this);
}

void CodeGenerator::visit(AssignmentNode &p_assignment) {

    lvalue = true;

    dumpInstrs("\n");
    dumpInstrs("# variable assignment: %s\n", p_assignment.getLvalue()->getNameCString());

    p_assignment.visitChildNodes(*this);


    dumpInstrs("%s%s%s%s%s%s%s", 
        "   lw t0, 0(sp)        # pop the value from the stack\n",
        "   addi sp, sp, 4\n",
        "   lw t1, 0(sp)        # pop the address from the stack\n",
        "   addi sp, sp, 4\n",
        "   sw t0, 0(t1)        # save the value to ",
        p_assignment.getLvalue()->getNameCString(),
        "\n"
    );

    dumpInstrs("\n");


    if(flag_for_assign){

        flag_for_assign = false;
        dumpInstrs("L%d:\n", label_now);

        int addr;    
        addr=hash_addr[p_assignment.getLvalue()->getName()].top();
        
        dumpInstrs("%s%d%s%s%s%s%s",
            "   lw t0, -",
            addr + 4,
            "(s0)      # load the value of ",
            p_assignment.getLvalue()->getNameCString(),
            "\n",
            "   addi sp, sp, -4\n",
            "   sw t0, 0(sp)        # push the value to the stack\n"
        );
        

    }
}

void CodeGenerator::visit(ReadNode &p_read) {
    dumpInstrs("\n");
    dumpInstrs("# read\n");

    lvalue = true;
    p_read.visitChildNodes(*this);


    dumpInstrs("%s%s%s%s%s%s", 
        "   jal ra, readInt     # call function 'readInt'\n",
        "   lw t0, 0(sp)        # pop the address from the stack\n",
        "   addi sp, sp, 4\n",
        "   sw a0, 0(t0)        # save the return value to ",
        p_read.getTarget()->getNameCString(),
        "\n\n"
    );

    
}

void CodeGenerator::visit(IfNode &p_if) {


    flag_if = true;
    main_function = false;
    flag_branch = true;

    label_base.push(label);
    label += 3;
    label_now = label_base.top();

    p_if.visitChildNodes(*this);

    flag_if = false;
    main_function = true;
    dumpInstrs("L%d:\n", label_now + 2);

    label_base.pop();
    if(label_base.size()!=0)
        label_now = label_base.top();
    else
        label_now = label;   
}

void CodeGenerator::visit(WhileNode &p_while) {

    label_base.push(label);
    label += 3;
    label_now = label_base.top();

    dumpInstrs("L%d:\n", label_now);

    flag_while = true;
    main_function = false;
    flag_branch = true;

    

    p_while.visitChildNodes(*this);

    flag_while = false;
    main_function = true;
    dumpInstrs("L%d:\n", label_now + 2);

    label_base.pop();

    if(label_base.size()!=0)
        label_now = label_base.top();
    else
        label_now = label;    
}

void CodeGenerator::visit(ForNode &p_for) {
    // Reconstruct the hash table for looking up the symbol entry
    symbol_manager->reconstructHashTableFromSymbolTable(p_for.getSymbolTable());

    label_base.push(label);
    label += 3;
    label_now = label_base.top();

    //local_addr = 8;
    main_function = false;
    flag_for = true;
    flag_for_assign = true;
    //flag_branch = true;

    p_for.visitChildNodes(*this);

    main_function = true;
    flag_for = false;

    int addr;
    addr=hash_addr[p_for.getInitialStatement()->getLvalue()->getName()].top();
    

    dumpInstrs("%s%d%s%s%s",
        "   addi t0, s0, -",
        addr + 4,
        "      # load the address of loop variable\n",
        "   addi sp, sp, -4\n",
        "   sw t0, 0(sp)        # push the address to the stack\n"

    );

    dumpInstrs("%s%d%s%s%s",
        "   lw t0, -",
        addr + 4,
        "(s0)      # load the value of loop variable\n",
        "   addi sp, sp, -4\n",
        "   sw t0, 0(sp)        # push the value to the stack\n"
    );


    dumpInstrs("%s%s%s", 
        "   li t0, 1\n",
        "   addi sp, sp, -4\n",
        "   sw t0, 0(sp)        # push the value to the stack\n"
    );

    dumpInstrs("%s%s%s%s", 
        "   lw t0, 0(sp)        # pop the value from the stack\n",
        "   addi sp, sp, 4\n",
        "   lw t1, 0(sp)        # pop the value from the stack\n",
        "   addi sp, sp, 4\n"
    );
    dumpInstrs("   add t0, t1, t0      # always save the value in a certain register you choose\n");

    dumpInstrs("%s%s%s%s%s%s%s", 
        "   addi sp, sp, -4\n",
        "   sw t0, 0(sp)        # push the value to the stack\n",
        "   lw t0, 0(sp)        # pop the value from the stack\n",
        "   addi sp, sp, 4\n",
        "   lw t1, 0(sp)        # pop the address from the stack\n",
        "   addi sp, sp, 4\n",
        "   sw t0, 0(t1)        # save the value to loop variable\n"

    );

    dumpInstrs("   j L%d                # jump back to loop condition\n", label_now);
    dumpInstrs("L%d:\n", label_now + 2);

    label_base.pop();

    if(label_base.size()!=0)
        label_now = label_base.top();
    else
        label_now = label;    


    if(p_for.getSymbolTable()!=nullptr){
        const auto &entries = (p_for.getSymbolTable())->getEntries();
        for (const auto &entry : entries) {

            if(hash_addr.find(entry->getName())!=hash_addr.end()){
                if(hash_addr[entry->getName()].size()==1){
                    hash_addr.erase(entry->getName());
                }
                else{
                    hash_addr[entry->getName()].pop();
                }
            }

        }
    }

    // Remove the entries in the hash table
    symbol_manager->removeSymbolsFromHashTable(p_for.getSymbolTable());
}

void CodeGenerator::visit(ReturnNode &p_return) {

    p_return.visitChildNodes(*this);

    dumpInstrs("\n%s%s%s", 
        "   lw t0, 0(sp)        # pop the value from the stack\n",
        "   addi sp, sp, 4\n",
        "   mv a0, t0           # load the value to the return value register 'a0'\n\n"
    );
}

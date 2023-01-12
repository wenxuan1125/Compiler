#include "AST/FunctionInvocation.hpp"
#include "visitor/AstNodeVisitor.hpp"

FunctionInvocationNode::FunctionInvocationNode(const uint32_t line,
                                               const uint32_t col,
                                               const char *p_name,
                                               Exprs *p_arguments)
    : ExpressionNode{line, col}, name(p_name),
      arguments(std::move(*p_arguments)) {
}

const char *FunctionInvocationNode::getNameCString() const {
    return name.c_str();
}

const int FunctionInvocationNode::getArgumentsNum() const{
    return arguments.size();
}


void FunctionInvocationNode::accept(AstNodeVisitor &p_visitor) {
    p_visitor.visit(*this);
}

void FunctionInvocationNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    for (auto &argument : arguments) {
        argument->accept(p_visitor);
    }
}

const int FunctionInvocationNode::checkInvalidChildren() const{
    for (int i = 0; i < arguments.size(); i++){
        if(std::strcmp(arguments[i]->getPTypeCString(), "null")==0)
            return 1;
    }

    return 0;
}

const int FunctionInvocationNode::checkArgumentsType( const char *parameters_type) const{

    char *split_string=new char[strlen(parameters_type) + 1];
    sprintf(split_string, "%s", parameters_type);
    
    char *parameter_type = strtok(split_string, ",");
    int i = 0; 
    while (parameter_type != NULL) {

        if(parameter_type[0]==' '){
            strcpy ( parameter_type, parameter_type+1);
        }

        if( std::strcmp(parameter_type, "real")==0){

            if( std::strcmp(arguments[i]->getPTypeCString(), "real")!=0 && 
                std::strcmp(arguments[i]->getPTypeCString(), "integer")!=0){

                return i;
            }

        }
        else if( std::strcmp(arguments[i]->getPTypeCString(), parameter_type)!=0){

            // printf("&&&&&\n");
            // printf("line %u: ", this->getLocation().line);
            // printf("parameter: %s\n", parameter_type);
            // printf("argument: %s\n",arguments[i]->getPTypeCString());
            // printf("&&&&&\n");
            return i;
        }
        
        i++;
        parameter_type = strtok(NULL, ",");
    }
    
    return -1;
}
const char *FunctionInvocationNode::getParameterTypeCString( const int id, const char *parameters_type) const{
    char *split_string=new char[strlen(parameters_type) + 1];
    sprintf(split_string, "%s", parameters_type);

    char *parameter_type = strtok(split_string, ",");
    int i = 0; 
    while (parameter_type != NULL) {

        if(parameter_type[0]==' '){
            strcpy ( parameter_type, parameter_type+1);
        }
        if( i==id)
            return parameter_type;

        i++;
        parameter_type = strtok(NULL, ",");
    }
}
const char *FunctionInvocationNode::getArgumentTypeCString( const int id) const{
    return arguments[id]->getPTypeCString();
}
const uint32_t FunctionInvocationNode::getArgumentLocationCol( const int id) const{
    return arguments[id]->getLocation().col;
}


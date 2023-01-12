# hw4 report

|||
|-:|:-|
|Name|成文瑄|
|ID|0716004|

## How much time did you spend on this project

e.g. 2 hours.

一個禮拜。

## Project overview

Describe the project structure and how you implemented it.

在這次的project中，利用了上次建好的ast，多新增了一種vistor（也就是semantic analyzer）去拜訪那個ast，然後在這個過程中，去建symbol table、做語意分析。

首先，我寫了我的symbol table會用到的東西的class ( 分別有symbol manager，是symbol table的stack、symbol table和symbol entry。然後class宣告好之後，就是開始建symbol table和做語意分析。

以program node的visit為例：

    current_table=new SymbolTable;
    current_entry = new SymbolEntry(p_program.getNameCString(), program, m_level, "void", "");
    current_table->addSymbol(current_entry);
    symbol_manager->pushScope(current_table);

    p_program.visitChildNodes(*this);

    current_table=symbol_manager->topScope();
    if(opt_dump)
        current_table->dumpSymbol();
    symbol_manager->popScope();

當我進入一個新的scope時，就要new一個新的symbol table然，然後在這個新的symbol table上加上這個program的symbol entry，接著把計算現在的level的全域變數++，然後去拜訪它的小孩。等拜訪完小孩之後，這個scope要結束了，所以如果現在沒有設定說不要印的話就把它印出來，然後再從stack中pop掉。在其他需要建symbol table有關的地方也是一樣的做法。

建好symbol table之後，再來是語意分析。首先做的事情是把expression node中的type設好（預設是nullptr，我在設type的時候，如果發現這顆node的小孩有錯，就不設這顆node的type，讓他維持是nullptr，所以在做語意分析時若要確認小孩有沒有錯，我就是以type是不是nullptr來確認的）。
所以以variable reference為例：

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

首先拜訪小孩，拜訪小孩回來之後check小孩有沒有error，沒有的話才繼續做下面的語意分析。因為在symbol table中會有一些entry是它宣告是錯的（像是測資中的err: integer[0]，array的宣告中，每個dimension都要大於0，雖然不合法，但它還是會放在symboltable中，所以要排除這種情況）。接著因為能拿來reference的變數並不一定是在目前的scope下宣告的，所以需要去現在的stack中把每張sybol table看過一次確認有沒有宣告過他（現在有在stack上的symbol table就是能reference的地方，像是live activation的感覺，我的做法是開temp的stack去存pop出來的symbol table，檢查完後再從這個暫時的pop出來push回原來的stack裡），全部找完都找不到就表示它是沒宣告過的，如果找到了的話，就去找到的那種symbol table中看它是不是variable、array的subscript個數是不是比宣告的多、suscript的值是不是integer，如果都沒錯的話，我就去symbol table中找這個variable的type然後給這個variable reference node設type。我在設type的時候把PType.hpp中的dimensions變數加上了mutable，因為當是array的時候，我也需要設這個變數，但因為我會在const的function設，所以要加上mutable。其他做語意分析的node也是差不多的做法。

另外，我印出code的地方是在scanner讀到\n時，把那行的code存入一個char*的array，在semantic analyzer中把這個array宣告成extern，要印出來的時候再用array[行數]，就可以拿到那行的code。

    /* Newline */
<INITIAL,CCOMMENT>\n {
    if (opt_src) {
        printf("%d: %s\n", line_num, buffer);
    }

    source_code[line_num] = strdup(buffer);
    ++line_num;
    col_num = 1;
    buffer[0] = '\0';
    buffer_ptr = buffer;
}


## What is the hardest you think in this project

Let us know, if any.

最難的部分我覺得是建表格和設expression node的type的地方。建表格因為有很多scope的事情要考慮，一開始還不是很清楚，花了一些時間在研究和修改它。type的話因為自己加了很多function，像是前面提到的把dimensions改成mutable或是為了的得到node在symbol table中的type做了很多字串處理，還滿辛苦的。

## Feedback to T.A.s

> Please help us improve our assignment, thanks.

小抱怨是明明是兩個星期的作業，但卻星期四才出，結果禮拜二就deadline了，寫起來壓力很大，而且感覺這次的作業難度應該也可以開到三個禮拜。

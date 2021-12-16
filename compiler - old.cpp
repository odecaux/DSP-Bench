/* date = November 15th 2021 1:55 pm */
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iterator>

#include <clang/Rewrite/Core/Rewriter.h>

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecordLayout.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Parse/ParseAST.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Frontend/FrontendActions.h>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/InitializePasses.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/DynamicLibrary.h>

#include "base.h"
#include "structs.h"

String allocate_and_copy_llvm_stringref(llvm::StringRef llvm_stringref)
{
    String new_string;
    new_string.size = llvm_stringref.size();
    new_string.str = new char[llvm_stringref.size()]; //TODO string type
    strcpy(new_string.str, llvm_stringref.data());
    return new_string;
}

static void print_parameter(Plugin_Descriptor_Parameter parameter)
{
    switch(parameter.type){
        case Int : {
            std::cout<<"Int type : min = " << parameter.int_param.min << ", max = " << parameter.int_param.max << "\n\n";
            
        }break;
        
        case Float : {
            std::cout<<"Float type : min = " << parameter.float_param.min << ", max = " << parameter.float_param.max << "\n\n";
            
        }break;
        
        case Enum : {
            std::cout<<"Enum type :\n";
            
            auto enum_param = parameter.enum_param;
            for(auto i = 0; i < enum_param.num_entries; i++)
            {
                //TODO si cest pas NULL-terminated ça va faire une grosse erreur
                std::cout << "   " << enum_param.entries[i].name.str << " : " << enum_param.entries[i].value << "\n";
            }
            std::cout << "\n";
        }break;
    }
}


clang::ast_matchers::DeclarationMatcher match_one_audio_callback()
{
    using namespace clang::ast_matchers;
    
    return functionDecl(hasName("audio_callback")).bind("callback");
}

class CallbackExists: public clang::ast_matchers::MatchFinder::MatchCallback
{
    public:
    bool& isValid;
    int count = 0;
    
    explicit CallbackExists(bool& isValid) : isValid{isValid} {}
    
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) {
        if(isValid == false) return;
        std::cout<<"is this a callback ?\n";
        
        const clang::FunctionDecl *declaration = Result.Nodes.getNodeAs<clang::FunctionDecl>("callback");
        if(declaration == nullptr) return;
        if(!declaration->isThisDeclarationADefinition()) return;
        
        std::cout<<"found one audio callback\n";
        count++;
    }
    
    virtual void onEndOfTranslationUnit()
    {
        if(count != 1)
        {
            isValid = false;
        }
    }
};



clang::ast_matchers::DeclarationMatcher match_right_audio_callback_parameters()
{
    using namespace clang::ast_matchers;
    
    return functionDecl(hasName("audio_callback"), 
                        parameterCountIs(4),
                        hasBody(compoundStmt()),
                        hasParameter(1, hasType(pointsTo(pointsTo(asString("float"))))),
                        hasParameter(2, varDecl(hasType(hasCanonicalType(allOf(isConstQualified(), isUnsignedInteger()))))),
                        hasParameter(3, varDecl(hasType(hasCanonicalType(allOf(isConstQualified(), isUnsignedInteger()))))),
                        returns(voidType())).bind("callback");
}


class CallbackHasRightParameters: public clang::ast_matchers::MatchFinder::MatchCallback
{
    bool already_found_a_callback = false;
    bool& isValid;
    std::string& found_struct_name;
    clang::SourceRange& range;
    
    public:
    
    CallbackHasRightParameters(bool& isValid, 
                               std::string& found_struct_name,
                               clang::SourceRange& range) : 
    isValid{isValid}, 
    found_struct_name{found_struct_name}, 
    range{range} {}
    
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) {
        if(!isValid) return;
        
        const clang::FunctionDecl *declaration = Result.Nodes.getNodeAs<clang::FunctionDecl>("callback");
        if(declaration == nullptr) return;
        if(!declaration->isThisDeclarationADefinition()) return;
        
        
        std::cout<<"found one valid audio callback\n";
        
        if(already_found_a_callback)
        {
            isValid = false;
            return;
        }
        
        
        already_found_a_callback = true;
        //found_callback_decl->dump();
        
        const auto& first_param_type = *(declaration->getParamDecl(0)->getType());
        assert(first_param_type.isReferenceType()); //TODO euh, ça doit être une vraie condition
        
        const auto& plugin_descriptor_type= *(first_param_type.getPointeeType());
        found_struct_name = plugin_descriptor_type.getAsCXXRecordDecl()->getCanonicalDecl()->getName().str();
        range = clang::SourceRange(declaration->getSourceRange());
    }
    
};

bool parse_plugin_descriptor(const clang::CXXRecordDecl* plugin_descriptor_declaration, Plugin_Descriptor& plugin_descriptor)
{
    if(plugin_descriptor_declaration->isPolymorphic())
    {
        std::cout<<"Cannot be polymorphic\n";
        return false;
    }
    
    const clang::ASTRecordLayout& layout  = plugin_descriptor_declaration->getASTContext().getASTRecordLayout(plugin_descriptor_declaration);
    auto size = layout.getSize();
    auto alignment = layout.getAlignment();
    
    
    plugin_descriptor.name = allocate_and_copy_llvm_stringref(plugin_descriptor_declaration->getName());
    plugin_descriptor.size = size.getQuantity();
    plugin_descriptor.alignment = alignment.getQuantity();
    
    u32 num_fields = 0;
    
    for(const auto* _ : plugin_descriptor_declaration->fields())
    {
        num_fields++;
    }
    
    auto *valid_parameters = (Plugin_Descriptor_Parameter*)malloc(sizeof(Plugin_Descriptor_Parameter) * num_fields);
    
    u32 valid_parameter_idx = 0;
    for(const auto* field : plugin_descriptor_declaration->fields())
    {
        const auto& type = *field->getType();
        
        if (field->hasAttr<clang::AnnotateAttr>()) {
            clang::AnnotateAttr* attr = field->getAttr<clang::AnnotateAttr>();
            
            std::string space_delimiter = " ";
            std::string text = attr->getAnnotation().str() ;
            std::vector<std::string> param_strings{};
            
            std::istringstream iss(text);
            
            std::string token;
            
            while(std::getline(iss, token, ' ')) {
                param_strings.push_back(token);
            }
            
            Plugin_Descriptor_Parameter plugin_parameter = {};
            plugin_parameter.name = allocate_and_copy_llvm_stringref(field->getName());
            
            auto index = field->getFieldIndex();
            plugin_parameter.offset = layout.getFieldOffset(index) / 8;
            
            if (param_strings[0] == "Int") {
                const auto *maybe_int = llvm::dyn_cast<clang::BuiltinType>(&type);
                if(maybe_int == nullptr || maybe_int->getKind() != clang::BuiltinType::Int)
                {
                    
                    std::cout<<"Error : not an Int type\n";
                    return false;
                }
                
                if(param_strings.size() != 3) 
                {
                    std::cout<<"two parameters required : min and max\n";
                    return false;
                }
                
                char* end_ptr = nullptr;
                i32 min = strtol(param_strings[1].c_str() , &end_ptr, 0);
                if(end_ptr == nullptr)
                {
                    std::cout<<"invalid min value\n";
                    return false;
                }
                i32 max = strtol(param_strings[2].c_str(), &end_ptr, 0);
                if(end_ptr == nullptr)
                {
                    std::cout<<"invalid max value\n";
                    return false;
                }
                if(min >= max)
                {
                    std::cout<<"min must be greater than max\n";
                    return false;
                }
                
                plugin_parameter.int_param.min = min;
                plugin_parameter.int_param.max = max; //TODO conversion;
                plugin_parameter.type = Int;
            }
            else if (param_strings[0] == "Float") 
            {
                
                const auto *maybe_float = llvm::dyn_cast<clang::BuiltinType>(&type);
                if(maybe_float == nullptr || maybe_float->getKind() != clang::BuiltinType::Float)
                {
                    
                    std::cout<<"Error : not a Float type\n";
                    return false;
                }
                
                if(param_strings.size() != 3) 
                {
                    std::cout<<"two parameters required : min and max\n";
                    return false;
                }
                
                char* end_ptr = nullptr;
                real32 min = strtof(param_strings[1].c_str(), &end_ptr);
                if(end_ptr == nullptr)
                {
                    std::cout<<"invalid min value\n";
                    return false;
                }
                real32 max = strtof(param_strings[2].c_str(), &end_ptr);
                if(end_ptr == nullptr)
                {
                    std::cout<<"invalid max value\n";
                    return false;
                }
                if(min >= max)
                {
                    std::cout<<"min must be greater than max\n";
                    return false;
                }
                
                plugin_parameter.float_param.min = min;
                plugin_parameter.float_param.max = max; //TODO conversion;
                plugin_parameter.type = Float;}
            else if (param_strings[0] == "Enum") 
            {
                const auto *maybe_enum = llvm::dyn_cast<clang::EnumType>(&type);
                if(maybe_enum == nullptr)
                {
                    
                    std::cout<<"Error : not an Enum type\n";
                    return false;
                }
                
                Parameter_Enum enum_param = {};
                
                auto *enum_decl = maybe_enum->getDecl();
                //TODO assert que le getIntegerType soit un int normal
                for(auto _: maybe_enum->getDecl()->enumerators())
                    enum_param.num_entries++;
                
                enum_param.entries = new Parameter_Enum_Entry[enum_param.num_entries];
                
                auto i = 0;
                for(const auto *field : maybe_enum->getDecl()->enumerators())
                {
                    enum_param.entries[i].name = allocate_and_copy_llvm_stringref(field->getName());
                    enum_param.entries[i].value = field->getInitVal().getExtValue(); //TODO conversion
                    i++;
                }
                
                plugin_parameter.type = Enum;
                plugin_parameter.enum_param = enum_param;
            }
            else {
                
                std::cout<<"Invalid type annotation, should be Int, Float or Enum\n";
                return false;
            }
            
            std::cout<<"\n";
            print_parameter(plugin_parameter);
            std::cout<<"\n";
            
            valid_parameters[valid_parameter_idx++] = plugin_parameter;
        }
    }
    
    valid_parameters = (Plugin_Descriptor_Parameter*) realloc(valid_parameters, sizeof(Plugin_Descriptor_Parameter) * valid_parameter_idx);
    
    plugin_descriptor.num_parameters = valid_parameter_idx;
    plugin_descriptor.parameters = valid_parameters;
    return true;
}

clang::ast_matchers::DeclarationMatcher match_any_struct()
{
    using namespace clang::ast_matchers;
    return cxxRecordDecl().bind("struct");
}


class Find_And_Parse_Plugin_Descriptor: public clang::ast_matchers::MatchFinder::MatchCallback
{
    public:
    bool& isValid;
    std::string plugin_descriptor_name;
    Plugin_Descriptor& parsed_plugin_descriptor;
    
    explicit Find_And_Parse_Plugin_Descriptor(bool& isValid, 
                                              std::string plugin_descriptor_name,
                                              Plugin_Descriptor& parsed_plugin_descriptor) : 
    isValid{isValid},
    plugin_descriptor_name{plugin_descriptor_name},
    parsed_plugin_descriptor{parsed_plugin_descriptor} 
    {}
    
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) {
        if(isValid == false) return;
        
        const clang::CXXRecordDecl *declaration = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("struct");
        if(declaration == nullptr) return;
        if(!declaration->isThisDeclarationADefinition()) return;
        if(declaration->getName() != plugin_descriptor_name) return;
        
        std::cout<<"found the struct \n";
        
        
        if(declaration->isPolymorphic())
        {
            isValid = false;
            return;
        }
        isValid = parse_plugin_descriptor(declaration, parsed_plugin_descriptor);
    }
};



clang::ast_matchers::DeclarationMatcher match_initialize_function()
{
    using namespace clang::ast_matchers;
    return functionDecl((hasName("initialize"))).bind("init");
}

class InitializeFunctionChecker : public clang::ast_matchers::MatchFinder::MatchCallback
{
    bool already_found_an_init_function = false;
    bool& isValid;
    std::string parameter_descriptor_name;
    clang::SourceRange& range;
    
    public :
    
    explicit InitializeFunctionChecker(bool& isValid, 
                                       std::string parameter_descriptor_name,
                                       clang::SourceRange& range) :
    isValid{isValid},
    parameter_descriptor_name{parameter_descriptor_name},
    range{range}
    {}
    
    
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &result)
    {
        if(!isValid) return;
        
        const clang::FunctionDecl *initialize_function = result.Nodes.getNodeAs<clang::FunctionDecl>("init");
        if(initialize_function == nullptr) return;
        if(!initialize_function->isThisDeclarationADefinition()) return;
        
        if(already_found_an_init_function == true)
        {
            isValid = false;
            return;
        }
        already_found_an_init_function = true;
        
        if(initialize_function->getNumParams() != 0)
        {
            isValid = false;
            std::cout<<"Initialize must not take any parameters\n";
            return;
        }
        
        auto found_parameter_name = initialize_function->getReturnType()->getAsCXXRecordDecl()->getCanonicalDecl()->getName().str();
        
        std::cout<<"struct : "<<parameter_descriptor_name<<", found : "<<found_parameter_name<<"\n";
        if(found_parameter_name != parameter_descriptor_name)
        {
            isValid = false;
            std::cout<<"Initialize return type does not match the audio callback parameter\n";
            return;
        }
        
        std::cout<<"found the initializer \n";
        
        
        range = clang::SourceRange(initialize_function->getSourceRange());
        
        return;
    }
    
};

std::string generate_init_wrapper(const std::string& parameter_name)
{
    return "\n"
        "void initialize_wrapper(void* void_out_initial_parameters)\n"
        "{\n"
        + parameter_name + "* out_initial_parameters = (" + parameter_name + "*)void_out_initial_parameters;\n"
        "*out_initial_parameters = initialize();\n"
        "}\n";
}


std::string generate_callback_wrapper(const std::string& parameter_name)
{
    return "\n"
        "void audio_callback_wrapper(void* data_ptr, float** out_buffer, const u32 num_channels," "const unsigned int num_samples)\n"
        "{\n"
        +parameter_name+"* data = (" + parameter_name + "*)data_ptr;\n"
        "audio_callback(*data, out_buffer, num_channels, num_samples);\n"
        "}\n";
}


extern "C" __declspec(dllexport) Plugin_Handle try_compile(const char* filename)
{
    
    int optimization_level = 3;
    int error_limit = 100;
    bool debug_mode = false;
    auto llvm = std::make_unique<llvm::LLVMContext>();
    
    //magic stuff
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    
    
    auto triple = llvm::sys::getDefaultTargetTriple();
    
    std::stringstream ss;
    ss << "-triple=" << triple;
    ss << " -O" << optimization_level;
    ss << " -fcxx-exceptions";
    ss << " -fms-extensions";
    
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::istream_iterator<std::string> i = begin;
    std::vector<const char*> itemcstrs;
    std::vector<std::string> itemstrs;
    for ( ; i != end; ++i) {
        itemstrs.push_back(*i);
    }
    for (unsigned idx = 0; idx < itemstrs.size(); idx++) {
        itemcstrs.push_back(itemstrs[idx].c_str());
    }
    
    // compiler instance.
    clang::CompilerInstance compilerInstance;
    compilerInstance.createDiagnostics();
    auto& compilerInvocation = compilerInstance.getInvocation();
    
    
    clang::CompilerInvocation::CreateFromArgs(compilerInvocation,
                                              itemcstrs, 
                                              compilerInstance.getDiagnostics());
    
    // Options.
    // auto& preprocessorOptions = compilerInvocation.getreprocessorOpts();
    auto& targetOptions = compilerInvocation.getTargetOpts();
    targetOptions.Triple = triple;
    
    auto* languageOptions = compilerInvocation.getLangOpts();
#ifdef _MSC_VER
    languageOptions->MSVCCompat = 1;
    languageOptions->MicrosoftExt = 1;
#endif
    languageOptions->Bool = 1;
    languageOptions->CPlusPlus = 1;
    
    auto& frontEndOptions = compilerInvocation.getFrontendOpts();
    frontEndOptions.Inputs.clear();
    
    auto kind = clang::InputKind(clang::Language::CXX);
    auto input = clang::FrontendInputFile(filename, kind);
    frontEndOptions.Inputs.push_back(input);
    
    auto& headerSearchOptions = compilerInvocation.getHeaderSearchOpts();
    
    if (debug_mode) {
        frontEndOptions.ShowStats = true;
        headerSearchOptions.Verbose = true;
    }
    
    
    bool isValid = true;
    std::string found_descriptor_name;
    Plugin_Descriptor parsed_descriptor_struct;
    CallbackExists callback_exists{isValid};
    clang::SourceRange callback_source_range {};
    clang::SourceRange initialize_source_range {};
    
    
    {
        CallbackHasRightParameters callback_has_right_parameters{isValid, found_descriptor_name, callback_source_range};
        clang::ast_matchers::MatchFinder match_finder;
        auto matcher_action_factory = clang::tooling::newFrontendActionFactory(&match_finder);
        auto matcher_action = matcher_action_factory->create();
        match_finder.addMatcher(match_one_audio_callback(), &callback_exists);
        match_finder.addMatcher(match_right_audio_callback_parameters(), &callback_has_right_parameters);
        compilerInstance.ExecuteAction(*matcher_action);
    }
    
    
    {
        Find_And_Parse_Plugin_Descriptor find_and_parse{isValid, found_descriptor_name, parsed_descriptor_struct};
        InitializeFunctionChecker init_checker{isValid, found_descriptor_name, initialize_source_range};
        
        clang::ast_matchers::MatchFinder match_finder;
        auto matcher_action_factory = clang::tooling::newFrontendActionFactory(&match_finder);
        auto matcher_action = matcher_action_factory->create();
        match_finder.addMatcher(match_initialize_function(), &init_checker);
        match_finder.addMatcher(match_any_struct(), &find_and_parse);
        compilerInstance.ExecuteAction(*matcher_action);
    }
    
    
    if(compilerInstance.getDiagnostics().getNumErrors() != 0)
        return {false};
    
    if(isValid == false)
    {
        return {false};
    }
    
    clang::Rewriter rewriter{compilerInstance.getSourceManager(), *languageOptions};
    
    rewriter.InsertTextBefore(callback_source_range.getBegin(), ""
                              "\nextern \"C\" void audio_callback_wrapper(void* data_ptr, float** out_buffer, const u32 num_channels, const unsigned int num_samples);\n"
                              "\n")
        ;
    auto callback_wrapper = generate_callback_wrapper(found_descriptor_name);
    rewriter.InsertTextAfter(callback_source_range.getEnd().getLocWithOffset(1), llvm::StringRef(callback_wrapper));
    
    
    rewriter.InsertTextBefore(initialize_source_range.getBegin(), "\nextern \"C\" void initialize_wrapper(void* void_out_initial_parameters);\n"
                              "");
    auto init_wrapper = generate_init_wrapper(found_descriptor_name);
    rewriter.InsertTextAfter(initialize_source_range.getEnd().getLocWithOffset(1), llvm::StringRef(init_wrapper));
    
    clang::SourceManager& source_manager = compilerInstance.getSourceManager();
    auto id = source_manager.getMainFileID();
    auto start_loc = source_manager.getLocForStartOfFile(id);
    auto end_loc = source_manager.getLocForEndOfFile(id);
    auto range = clang::SourceRange(start_loc, end_loc);
    
    auto compile_action = std::make_unique<clang::EmitLLVMOnlyAction>(llvm.get());
    auto new_text = rewriter.getRewrittenText(range);
    auto new_buffer = llvm::MemoryBuffer::getMemBuffer(new_text);
    
    auto new_input_file = clang::FrontendInputFile(new_buffer->getMemBufferRef(), kind);
    
    frontEndOptions.Inputs.clear();
    frontEndOptions.Inputs.push_back(new_input_file);
    
    
    
    if (compilerInstance.ExecuteAction(*compile_action)) 
    {
        printf("tried compiling\n");
        if(compilerInstance.getDiagnostics().getNumErrors() == 0)
        {
            std::cout << "compilation worked\n";
            
            
            std::unique_ptr<llvm::Module> module = compile_action->takeModule();
            
            if(module)
            {
                printf("module loaded\n");
                auto& codeGenOptions = compilerInvocation.getCodeGenOpts();
                // optimizations.
                llvm::PassBuilder passBuilder;
                llvm::LoopAnalysisManager loopAnalysisManager;
                llvm::FunctionAnalysisManager functionAnalysisManager;
                llvm::CGSCCAnalysisManager cGSCCAnalysisManager;
                llvm::ModuleAnalysisManager moduleAnalysisManager;
                
                //functionAnalysisManager.registerass([&]{ return passBuilder.buildDefaultAAipeline(); });
                
                passBuilder.registerModuleAnalyses(moduleAnalysisManager);
                passBuilder.registerCGSCCAnalyses(cGSCCAnalysisManager);
                passBuilder.registerFunctionAnalyses(functionAnalysisManager);
                passBuilder.registerLoopAnalyses(loopAnalysisManager);
                passBuilder.crossRegisterProxies(loopAnalysisManager, functionAnalysisManager, cGSCCAnalysisManager, moduleAnalysisManager);
                
                llvm::ModulePassManager modulePassManager = passBuilder.buildPerModuleDefaultPipeline(llvm::PassBuilder::OptimizationLevel::O3);
                modulePassManager.run(*module, moduleAnalysisManager);
                
                
                llvm::EngineBuilder builder(std::move(module));
                builder.setMCJITMemoryManager(std::make_unique<llvm::SectionMemoryManager>());
                builder.setOptLevel(llvm::CodeGenOpt::Level::Aggressive);
                
                
                auto& Registry = *llvm::PassRegistry::getPassRegistry();
                llvm::initializeCore(Registry);
                llvm::initializeScalarOpts(Registry);
                llvm::initializeVectorization(Registry);
                llvm::initializeIPO(Registry);
                llvm::initializeAnalysis(Registry);
                llvm::initializeTransformUtils(Registry);
                llvm::initializeInstCombine(Registry);
                llvm::initializeInstrumentation(Registry);
                llvm::initializeTarget(Registry);
                
                llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
                
                llvm::ExecutionEngine *engine = builder.create();
                
                
                if (engine) 
                {
                    engine->finalizeObject();
                    
                    
                    
                    typedef void(*callback_t)(void*, float**, unsigned int, unsigned int);
                    auto callback = (callback_t)engine->getFunctionAddress("audio_callback_wrapper");
                    
                    typedef void(*initializer_t)(void*);
                    auto initializer = (initializer_t)engine->getFunctionAddress("initialize_wrapper");
                    
                    
                    if (callback && initializer) {
                        return Plugin_Handle{true, callback, initializer, parsed_descriptor_struct};
                    }
                    else
                    {
                        printf("didn't find ff\n");
                        return Plugin_Handle{false};
                    }
                    
                }
                
            }
            
        }
    }
    return {false};
}
////////////////////////////////////////////////////////////////////////////
// Clang Driver needs version.dll for MSVC
////////////////////////////////////////////////////////////////////////////
#if _WIN32
#pragma comment(lib, "version.lib")

////////////////////////////////////////////////////////////////////////////
// Clang library


#pragma comment(lib, "clangAnalysis.lib")
#pragma comment(lib, "clangAPINotes.lib")
#pragma comment(lib, "clangARCMigrate.lib")
#pragma comment(lib, "clangAST.lib")
#pragma comment(lib, "clangASTMatchers.lib")
#pragma comment(lib, "clangBasic.lib")
#pragma comment(lib, "clangCodeGen.lib")
#pragma comment(lib, "clangCrossTU.lib")
#pragma comment(lib, "clangDependencyScanning.lib")
#pragma comment(lib, "clangDirectoryWatcher.lib")
#pragma comment(lib, "clangDriver.lib")
#pragma comment(lib, "clangAstMatchers.lib")
//#pragma comment(lib, "clangDynamicASTMatchers.lib")
#pragma comment(lib, "clangEdit.lib")
//#pragma comment(lib, "clangFormat.lib")
#pragma comment(lib, "clangFrontend.lib")
#pragma comment(lib, "clangFrontendTool.lib")
#pragma comment(lib, "clangHandleCXX.lib")
#pragma comment(lib, "clangHandleLLVM.lib")
#pragma comment(lib, "clangIndex.lib")
#pragma comment(lib, "clangIndexSerialization.lib")
//#pragma comment(lib, "clangInterpreter.lib")
#pragma comment(lib, "clangLex.lib")
#pragma comment(lib, "clangParse.lib")
#pragma comment(lib, "clangRewrite.lib")
#pragma comment(lib, "clangRewriteFrontend.lib")
#pragma comment(lib, "clangSema.lib")
#pragma comment(lib, "clangSerialization.lib")
#pragma comment(lib, "clangStaticAnalyzerCheckers.lib")
#pragma comment(lib, "clangStaticAnalyzerCore.lib")
#pragma comment(lib, "clangStaticAnalyzerFrontend.lib")
//#pragma comment(lib, "clangTesting.lib")
#pragma comment(lib, "clangTooling.lib")
#pragma comment(lib, "clangToolingASTDiff.lib")
#pragma comment(lib, "clangToolingCore.lib")
#pragma comment(lib, "clangToolingInclusions.lib")
#pragma comment(lib, "clangToolingRefactoring.lib")
#pragma comment(lib, "clangToolingSyntax.lib")
#pragma comment(lib, "clangTransformer.lib")
#pragma comment(lib, "libclang.lib")
#pragma comment(lib, "lldCOFF.lib")
#pragma comment(lib, "lldCommon.lib")
#pragma comment(lib, "lldCore.lib")
#pragma comment(lib, "lldDriver.lib")
//#pragma comment(lib, "lldELF.lib")
#pragma comment(lib, "lldMachO.lib")
#pragma comment(lib, "lldMachO2.lib")
//#pragma comment(lib, "lldMinGW.lib")
#pragma comment(lib, "lldReaderWriter.lib")
//#pragma comment(lib, "lldWasm.lib")
#pragma comment(lib, "lldYAML.lib")
//#pragma comment(lib, "LLVM-C.lib")
#pragma comment(lib, "LLVMAggressiveInstCombine.lib")

#pragma comment(lib, "LLVMAnalysis.lib")

#pragma comment(lib, "LLVMAsmParser.lib")
#pragma comment(lib, "LLVMAsmPrinter.lib")

#pragma comment(lib, "LLVMBinaryFormat.lib")
#pragma comment(lib, "LLVMBitReader.lib")
#pragma comment(lib, "LLVMBitstreamReader.lib")
#pragma comment(lib, "LLVMBitWriter.lib")

#pragma comment(lib, "LLVMCFGuard.lib")
#pragma comment(lib, "LLVMCFIVerify.lib")
#pragma comment(lib, "LLVMCodeGen.lib")
#pragma comment(lib, "LLVMCore.lib")
#pragma comment(lib, "LLVMCoroutines.lib")
#pragma comment(lib, "LLVMCoverage.lib")
#pragma comment(lib, "LLVMDebugInfoCodeView.lib")
#pragma comment(lib, "LLVMDebugInfoDWARF.lib")
#pragma comment(lib, "LLVMDebugInfoGSYM.lib")
#pragma comment(lib, "LLVMDebugInfoMSF.lib")
#pragma comment(lib, "LLVMDebugInfoPDB.lib")
#pragma comment(lib, "LLVMDemangle.lib")
#pragma comment(lib, "LLVMDlltoolDriver.lib")
#pragma comment(lib, "LLVMDWARFLinker.lib")
#pragma comment(lib, "LLVMDWP.lib")
#pragma comment(lib, "LLVMExecutionEngine.lib")

#pragma comment(lib, "LLVMExtensions.lib")
#pragma comment(lib, "LLVMFileCheck.lib")
#pragma comment(lib, "LLVMFrontendOpenACC.lib")
#pragma comment(lib, "LLVMFrontendOpenMP.lib")
#pragma comment(lib, "LLVMFuzzMutate.lib")
#pragma comment(lib, "LLVMGlobalISel.lib")

#pragma comment(lib, "LLVMInstCombine.lib")
#pragma comment(lib, "LLVMInstrumentation.lib")
#pragma comment(lib, "LLVMInterfaceStub.lib")
#pragma comment(lib, "LLVMInterpreter.lib")
#pragma comment(lib, "LLVMipo.lib")
#pragma comment(lib, "LLVMIRReader.lib")
#pragma comment(lib, "LLVMJITLink.lib")

#pragma comment(lib, "LLVMLibDriver.lib")
#pragma comment(lib, "LLVMLineEditor.lib")
#pragma comment(lib, "LLVMLinker.lib")
#pragma comment(lib, "LLVMLTO.lib")
#pragma comment(lib, "LLVMMC.lib")
#pragma comment(lib, "LLVMMCA.lib")
#pragma comment(lib, "LLVMMCACustomBehaviourAMDGPU.lib")
#pragma comment(lib, "LLVMMCDisassembler.lib")
#pragma comment(lib, "LLVMMCJIT.lib")
#pragma comment(lib, "LLVMMCParser.lib")

#pragma comment(lib, "LLVMMIRParser.lib")


#pragma comment(lib, "LLVMObjCARCOpts.lib")
#pragma comment(lib, "LLVMObject.lib")
#pragma comment(lib, "LLVMObjectYAML.lib")
#pragma comment(lib, "LLVMOption.lib")
#pragma comment(lib, "LLVMOrcJIT.lib")
#pragma comment(lib, "LLVMOrcShared.lib")
#pragma comment(lib, "LLVMOrcTargetProcess.lib")


#pragma comment(lib, "LLVMPasses.lib")

#pragma comment(lib, "LLVMProfileData.lib")
#pragma comment(lib, "LLVMRemarks.lib")

#pragma comment(lib, "LLVMRuntimeDyld.lib")
#pragma comment(lib, "LLVMScalarOpts.lib")
#pragma comment(lib, "LLVMSelectionDAG.lib")
#pragma comment(lib, "LLVMSupport.lib")
#pragma comment(lib, "LLVMSymbolize.lib")

#pragma comment(lib, "LLVMTableGen.lib")
#pragma comment(lib, "LLVMTableGenGlobalISel.lib")
#pragma comment(lib, "LLVMTarget.lib")
#pragma comment(lib, "LLVMTextAPI.lib")
#pragma comment(lib, "LLVMTransformUtils.lib")
#pragma comment(lib, "LLVMVectorize.lib")

#pragma comment(lib, "LLVMWindowsManifest.lib")

#pragma comment(lib, "LTO.lib")




#pragma comment(lib, "LLVMX86AsmParser.lib")
#pragma comment(lib, "LLVMX86CodeGen.lib")
#pragma comment(lib, "LLVMX86Desc.lib")
#pragma comment(lib, "LLVMX86Disassembler.lib")
#pragma comment(lib, "LLVMX86Info.lib")

#endif //MSVC

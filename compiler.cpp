/* date = November 15th 2021 1:55 pm */
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iterator>

#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Sema/Sema.h"


#include "clang/Serialization/PCHContainerOperations.h"
#include <clang/Rewrite/Core/Rewriter.h>
#include "clang/Basic/Builtins.h"
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

#include "clang/Frontend/TextDiagnosticPrinter.h"
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
#include "llvm/Analysis/AliasAnalysis.h"

#include "base.h"
#include "structs.h"

struct Clang_Context{
    llvm::ModulePassManager module_pass_manager;
    llvm::ModuleAnalysisManager module_analysis_manager;
};


extern "C" __declspec(dllexport) void* create_clang_context()
{
    llvm::PassBuilder passBuilder;
    llvm::LoopAnalysisManager loopAnalysisManager;
    llvm::FunctionAnalysisManager functionAnalysisManager;
    llvm::CGSCCAnalysisManager cGSCCAnalysisManager;
    llvm::ModuleAnalysisManager moduleAnalysisManager;
    
    
    passBuilder.registerModuleAnalyses(moduleAnalysisManager);
    passBuilder.registerCGSCCAnalyses(cGSCCAnalysisManager);
    passBuilder.registerFunctionAnalyses(functionAnalysisManager);
    functionAnalysisManager.registerPass([&]{ return passBuilder.buildDefaultAAPipeline(); });
    passBuilder.registerLoopAnalyses(loopAnalysisManager);
    passBuilder.crossRegisterProxies(loopAnalysisManager, functionAnalysisManager, cGSCCAnalysisManager, moduleAnalysisManager);
    
    llvm::ModulePassManager module_pass_manager = passBuilder.buildPerModuleDefaultPipeline(llvm::PassBuilder::OptimizationLevel::O3);
    
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
    
    
    auto* clang_ctx = new Clang_Context();
    clang_ctx->module_pass_manager = std::move(module_pass_manager);
    clang_ctx->module_analysis_manager = std::move(moduleAnalysisManager);
    
    return (void*) clang_ctx;
}

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



template<typename Exec>
class Consumer : public clang::ASTConsumer
{
    public:
    explicit Consumer(Exec&& exec) : exec(exec), clang::ASTConsumer() { }
    
    virtual void HandleTranslationUnit(clang::ASTContext &Context)
    {
        exec(Context);
    }
    
    Exec exec;
};

template<typename Exec>
Consumer<Exec> make_consumer(Exec&&  exec)
{
    return Consumer<Exec>(exec); 
}


template<typename Exec>
class MatchCallback : public clang::ast_matchers::MatchFinder::MatchCallback
{
    public:
    explicit MatchCallback(Exec exec) : exec(exec) { }
    
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult& result)
    {
        exec(result);
    }
    
    Exec exec;
};


template<typename Exec>
MatchCallback<Exec> make_match_callback(Exec&&  exec)
{
    return MatchCallback<Exec>(exec); 
}

template<typename Matcher, typename Exec>
void match_ast(Matcher& matcher, Exec& exec, clang::ASTContext& ast_ctx)
{
    using namespace clang::ast_matchers;
    
    auto match_finder = MatchFinder{};
    
    
    auto matcher_callback = make_match_callback(exec);
    match_finder.addMatcher(matcher, &matcher_callback);
    match_finder.matchAST(ast_ctx);
} 

template<typename Matcher, typename Exec, typename Node>
void match_node(Matcher& matcher, Exec& exec, const Node& node, clang::ASTContext& ast_ctx)
{
    using namespace clang::ast_matchers;
    
    auto match_finder = MatchFinder{};
    
    
    auto matcher_callback = make_match_callback(exec);
    match_finder.addMatcher(matcher, &matcher_callback);
    match_finder.match<Node>(node, ast_ctx);
} 

bool parse_plugin_descriptor(const clang::CXXRecordDecl* parameters_struct_decl, 
                             const clang::CXXRecordDecl* state_struct_decl,
                             Plugin_Descriptor& plugin_descriptor);

extern "C" __declspec(dllexport) Plugin_Handle try_compile(const char* filename, void* clang_ctx_ptr)
{
    
    //magic stuff
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    
    clang::DiagnosticOptions diagnosticOptions;
    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> diagnostics_ids;
    
    clang::TextDiagnosticPrinter *text_diagnostics_printer =
        new clang::TextDiagnosticPrinter(
                                         llvm::outs(),
                                         &diagnosticOptions);
    
    clang::DiagnosticsEngine diagnostics_engine (diagnostics_ids,
                                                 &diagnosticOptions,
                                                 text_diagnostics_printer);
    
    clang::LangOptions language_options;
    language_options.Bool = 1;
    language_options.CPlusPlus = 1;
    language_options.RTTI = 0;
    language_options.CXXExceptions = 0;
    
    clang::FileSystemOptions filesystem_options;
    clang::FileManager file_manager(filesystem_options);
    
    std::shared_ptr<clang::HeaderSearchOptions> header_searchOptions(new clang::HeaderSearchOptions());
    
    const std::shared_ptr<clang::TargetOptions> targetOptions = std::make_shared<clang::TargetOptions>();
    targetOptions->Triple = llvm::sys::getDefaultTargetTriple();
    
    clang::TargetInfo *target_infos = 
        clang::TargetInfo::CreateTargetInfo(
                                            diagnostics_engine,
                                            targetOptions);
    
    //~
    clang::SourceManager source_manager(diagnostics_engine,
                                        file_manager);
    
    
    clang::HeaderSearch header_search(header_searchOptions,
                                      source_manager, 
                                      diagnostics_engine,
                                      language_options,
                                      target_infos);
    
    clang::CompilerInstance compiler_instance;
    
    std::shared_ptr<clang::PreprocessorOptions> preprocessor_options( new clang::PreprocessorOptions());
    clang::Preprocessor preprocessor(preprocessor_options,
                                     diagnostics_engine,
                                     language_options,
                                     source_manager,
                                     header_search,
                                     compiler_instance);
    preprocessor.Initialize(*target_infos);
    
    clang::PCHContainerOperations pch_container{};
    
    clang::FrontendOptions frontend_options;
    
    clang::InitializePreprocessor(preprocessor,
                                  *preprocessor_options,
                                  pch_container.getRawReader(),
                                  frontend_options);
    
    clang::ApplyHeaderSearchOptions( preprocessor.getHeaderSearchInfo(),
                                    compiler_instance.getHeaderSearchOpts(),
                                    preprocessor.getLangOpts(),
                                    preprocessor.getTargetInfo().getTriple());
    
    
    const clang::TargetInfo &targetInfo = *target_infos;
    
    clang::IdentifierTable identifier_table(language_options);
    clang::SelectorTable selector_table;
    
    clang::Builtin::Context builtin_context;
    builtin_context.InitializeTarget(targetInfo, nullptr);
    
    //~
    //plugin source
    llvm::ErrorOr<const clang::FileEntry *> plugin_source_file_handle = file_manager.getFile(filename);
    if(!plugin_source_file_handle)
    {
        std::cout<<"invalid source file\n";
        return {false};
    }
    auto plugin_source_file_id = source_manager.createFileID(*plugin_source_file_handle, clang::SourceLocation(), clang::SrcMgr::C_User);
    source_manager.setMainFileID(plugin_source_file_id);
    
    
    //~AST CONTEXT
    
    clang::ASTContext plugin_source_ast_context(language_options,
                                                source_manager,
                                                identifier_table,
                                                selector_table,
                                                builtin_context,
                                                clang::TU_Complete);
    
    plugin_source_ast_context.InitBuiltinTypes(*target_infos);
    
    //~
    
    Plugin_Descriptor descriptor;
    std::unique_ptr<llvm::MemoryBuffer> new_buffer = nullptr;
    
    bool error = false;
    
    auto visit_ast = [&](clang::ASTContext& ast_ctx){
        
        using namespace clang::ast_matchers;
        
        //~ does functions exists ?
        
        const clang::FunctionDecl* audio_callback_decl = nullptr;
        
        auto audio_callback_matcher = functionDecl(hasName("audio_callback")).bind("callback");
        auto audio_callback_lambda = [&](const auto& result)
        {
            if(error) return; 
            
            const clang::FunctionDecl* decl = result.Nodes.getNodeAs<clang::FunctionDecl>("callback");
            
            if(decl == nullptr) return;
            if(!decl->isThisDeclarationADefinition()) return;
            if(audio_callback_decl == nullptr)
            {
                audio_callback_decl = decl;
            }
            else
            {
                std::cout<<"too many audio_callback function\n";
                error = true;
            }
        };
        match_ast(audio_callback_matcher, audio_callback_lambda, ast_ctx);
        
        if(audio_callback_decl == nullptr)
        {
            error = true;
            std::cout<<"couldn't find an audio_callback function\n";
            return;
        }
        
        
        if(error)
            return;
        
        const clang::FunctionDecl* default_parameters_decl = nullptr;
        
        auto default_parameters_matcher = functionDecl(hasName("default_parameters")).bind("default_param");
        auto default_parameters_lambda = [&](const auto& result)
        {
            if(error) return; 
            
            const clang::FunctionDecl* decl = result.Nodes.getNodeAs<clang::FunctionDecl>("default_param");
            
            if(decl == nullptr) return;
            if(!decl->isThisDeclarationADefinition()) return;
            if(default_parameters_decl == nullptr)
            {
                default_parameters_decl = decl;
            }
            else
            {
                std::cout<<"too many default_parameters function\n";
                error = true;
            }
        };
        match_ast(default_parameters_matcher, default_parameters_lambda, ast_ctx);
        
        if(default_parameters_decl == nullptr)
        {
            error = true;
            std::cout<<"couldn't find an default_parameters function\n";
            return;
        }
        if(error)
            return;
        
        
        
        const clang::FunctionDecl* initialize_state_decl = nullptr;
        
        auto initialize_state_matcher = functionDecl(hasName("initialize_state")).bind("initialize_state");
        auto initialize_state_lambda = [&](const auto& result)
        {
            if(error) return; 
            
            const clang::FunctionDecl* decl = result.Nodes.getNodeAs<clang::FunctionDecl>("initialize_state");
            
            if(decl == nullptr) return;
            if(!decl->isThisDeclarationADefinition()) return;
            if(initialize_state_decl == nullptr)
            {
                initialize_state_decl = decl;
            }
            else
            {
                std::cout<<"too many initialize_state function\n";
                error = true;
            }
        };
        match_ast(initialize_state_matcher, initialize_state_lambda, ast_ctx);
        
        if(initialize_state_decl == nullptr)
        {
            error = true;
            std::cout<<"couldn't find an initialize_state function\n";
            return;
        }
        if(error)
            return;
        
        //~ audio callback has the right signature
        
        auto audio_callback_has_right_signature_matcher = 
            functionDecl(hasName("audio_callback"), 
                         parameterCountIs(6),
                         hasBody(compoundStmt()),
                         
                         hasParameter(2, hasType(pointsTo(pointsTo(realFloatingPointType())))),
                         hasParameter(3, varDecl(hasType(hasCanonicalType(allOf(isConstQualified(), isUnsignedInteger()))))),
                         hasParameter(4, varDecl(hasType(hasCanonicalType(allOf(isConstQualified(), isUnsignedInteger()))))),
                         hasParameter(5, varDecl(hasType(hasCanonicalType(allOf(isConstQualified(), realFloatingPointType()))))),
                         
                         returns(voidType()))
            .bind("callback");
        
        bool has_audio_callback_the_right_signature = false;
        
        auto audio_callback_has_right_signature_lambda = [&](const auto& result)
        {
            if(error) return; 
            const clang::FunctionDecl* decl = result.Nodes.getNodeAs<clang::FunctionDecl>("callback");
            if(decl == nullptr) return;
            if(!decl->isThisDeclarationADefinition()) return;
            
            if(has_audio_callback_the_right_signature == false)
            {
                has_audio_callback_the_right_signature = true;
            }
            else
            {
                error = true;
            }
        };
        
        
        match_node(audio_callback_has_right_signature_matcher,
                   audio_callback_has_right_signature_lambda,
                   *audio_callback_decl,
                   ast_ctx);
        
        
        if(has_audio_callback_the_right_signature == false)
        {
            std::cout << "audio_callback doesn't have the right signature\n";
            return;
        }
        if(error) return;
        
        
        //~
        // check initializer signature
        
        bool initializer_has_the_right_signature = [&](){
            
            auto num_params = initialize_state_decl->getNumParams();
            if(num_params != 4)
            {
                std::cout<<"invalid parameter count\n";
                return false;
            }
            
            //TODO check que c'est ni const ni volatile
            const auto& parameters_type = *initialize_state_decl->getParamDecl(0)->getType().getCanonicalType();
            const auto& num_channels_type = *initialize_state_decl->getParamDecl(1)->getType().getCanonicalType();
            const auto& sample_rate_type = *initialize_state_decl->getParamDecl(2)->getType().getCanonicalType();
            const auto& allocator_type = *initialize_state_decl->getParamDecl(3)->getType().getCanonicalType();
            
            
            
            if(!num_channels_type.isSpecificBuiltinType(clang::BuiltinType::UInt))
            {
                std::cout << "second parameter is unsigned int num_channels\n";
                return false;
            }
            
            //TODO assert que float est 32 bit
            if(!sample_rate_type.isSpecificBuiltinType(clang::BuiltinType::Float))
            {
                std::cout << "third parameter is float sample_rate\n";
                return false;
            }
            
            auto allocator_has_the_right_signature = [&]() -> bool {
                if(!allocator_type.isFunctionPointerType()) return false;
                const auto* pt = llvm::dyn_cast<clang::PointerType>(&allocator_type);
                const auto* ft = pt->getPointeeType()->getAs<clang::FunctionProtoType>();
                
                auto allocator_num_params = ft->getNumParams();
                if(allocator_num_params != 1) return false;
                
                const auto& num_samples_param_type = *ft->getParamType(0);
                if(!num_samples_param_type.isSpecificBuiltinType(clang::BuiltinType::UInt)) return false;
                
                if(!ft->getReturnType()->isVoidPointerType()) return false;
                
                return true;
            }();
            
            if(!allocator_has_the_right_signature)
            {
                std::cout<<"fourth parameter should be allocator (unsigned int num_samples) -> void\n"; 
                return false;
            }
            return true;
        }();
        
        if(!initializer_has_the_right_signature)
        {
            error = true;
            return;
        }
        
        if(default_parameters_decl->getNumParams() != 0)
        {
            std::cout<<"default_parameters doesn't take any parameters\n";
            error = true;
            return;
        }
        
        //~comparer les types
        
        const auto& default_parameters_return_type = *default_parameters_decl->getReturnType();
        const auto& initialize_state_return_type = *initialize_state_decl->getReturnType();
        
        //TODO si c'est pas des pointers, alors ça va être pas ouf
        
        const auto& initialize_state_parameter_type = *initialize_state_decl->getParamDecl(0)->getType()->getPointeeType();
        const auto& audio_callback_parameter_type = *audio_callback_decl->getParamDecl(0)->getType()->getPointeeType();
        const auto& audio_callback_state_type = *audio_callback_decl->getParamDecl(1)->getType()->getPointeeType();
        
        if(&default_parameters_return_type != &initialize_state_parameter_type ||
           &default_parameters_return_type != &audio_callback_parameter_type)
        {
            std::cout<<"parameter type don't match\n";
            error = true;
            return;
        }
        
        if(&initialize_state_return_type != &audio_callback_state_type)
        {
            std::cout << "state types don't match\n";
            error = true;
            return;
        }
        
        const auto& state_type = initialize_state_return_type;
        const auto& parameters_type = default_parameters_return_type;
        
        if(!state_type.isRecordType() || !parameters_type.isRecordType())
        {
            std::cout<< "must be records\n"; 
            error = true;
            return;
        }
        
        const auto* state_struct_decl = state_type.getAsCXXRecordDecl();
        const auto* parameters_struct_decl = parameters_type.getAsCXXRecordDecl();
        
        parse_plugin_descriptor(parameters_struct_decl, state_struct_decl, descriptor); 
        
        //~ rewrite the source file
        
        auto audio_callback_source_range = audio_callback_decl->getSourceRange();
        auto default_parameters_source_range = default_parameters_decl->getSourceRange();
        auto initialize_state_source_range = initialize_state_decl->getSourceRange();
        
        auto parameters_struct_name = parameters_struct_decl->getNameAsString();
        auto state_struct_name = state_struct_decl->getNameAsString();
        
        std::string audio_callback_wrapper_declaration = ""
            "\nextern \"C\" void audio_callback_wrapper(void* param_ptr, void* state_ptr,float** out_buffer, unsigned int num_channels, unsigned int num_samples, float sample_rate);\n"
            "\n";
        
        std::string audio_callback_wrapper_definition = "\n"
            "void audio_callback_wrapper(void* param_ptr, void* state_ptr,float** out_buffer, unsigned int num_channels, unsigned int num_samples, float sample_rate)\n"
            "{\n"
            + parameters_struct_name  +"* param  = (" + parameters_struct_name + "*)param_ptr;\n"
            + state_struct_name  +"* state  = (" + state_struct_name + "*)state_ptr;\n"
            "audio_callback(*param, *state, out_buffer, num_channels, num_samples, sample_rate);\n"
            "}\n";
        
        std::string default_parameters_wrapper_declaration = "\nextern \"C\" void default_parameters_wrapper(void* out_parameters);\n"
            "";
        
        std::string default_parameters_wrapper_definition = "\n"
            "void default_parameters_wrapper(void* out_parameters_ptr)\n"
            "{\n"
            + parameters_struct_name + "* out_parameters = (" + parameters_struct_name + "*)void_out_parameters_ptr;\n"
            "*out_initial_parameters = default_parameters();\n"
            "}\n";
        
        std::string initialize_state_wrapper_declaration = "\nextern \"C\" void initialize_state_wrapper(void* parameters, void* out_initial_state);\n"
            "";
        
        std::string initialize_state_wrapper_definition = "\n"
            "void initialize_state_wrapper(void* parameters_ptr, void* out_initial_state_ptr)\n"
            "{\n"
            + parameters_struct_name + "* parameters = (" + parameters_struct_name + "*)parameters_ptr;\n"
            
            + state_struct_name + "* out_initial_state = (" + state_struct_name + "*)out_initial_state_ptr;\n"
            "*out_initial_state = initialize_state(*parameters);\n"
            "}\n";
        
        
        clang::Rewriter rewriter{source_manager, language_options};
        
        
        rewriter.InsertTextBefore(audio_callback_source_range.getBegin(), audio_callback_wrapper_declaration);
        rewriter.InsertTextAfter(audio_callback_source_range.getEnd().getLocWithOffset(1), audio_callback_wrapper_definition);
        
        
        rewriter.InsertTextBefore(default_parameters_source_range.getBegin(), default_parameters_wrapper_declaration);
        rewriter.InsertTextAfter(default_parameters_source_range.getEnd().getLocWithOffset(1), default_parameters_wrapper_definition);
        
        rewriter.InsertTextBefore(initialize_state_source_range.getBegin(), initialize_state_wrapper_declaration);
        rewriter.InsertTextAfter(initialize_state_source_range.getEnd().getLocWithOffset(1),  initialize_state_wrapper_definition);
        
        auto start_loc = source_manager.getLocForStartOfFile(plugin_source_file_id);
        auto end_loc = source_manager.getLocForEndOfFile(plugin_source_file_id);
        auto range = clang::SourceRange(start_loc, end_loc);
        
        auto new_text = rewriter.getRewrittenText(range);
        new_buffer = llvm::MemoryBuffer::getMemBuffer(new_text);
        
        std::cout<< "\n\n\n";
        std::cout<< new_text;
    };
    
    
    auto astConsumer = make_consumer(visit_ast);
    
    clang::Sema sema(preprocessor,
                     plugin_source_ast_context,
                     astConsumer);
    
    text_diagnostics_printer->BeginSourceFile(language_options, &preprocessor);
    clang::ParseAST(sema); 
    text_diagnostics_printer->EndSourceFile();
    
    //TODO necessary ?
    //plugin_source_ast_context.cleanup();
    
    if(error == true)
    {
        std::cout<<"error while parsing file\n";
        return {false};
    }
    
    //~ JIT compilation
    
    source_manager.overrideFileContents(*plugin_source_file_handle, std::move(new_buffer));
    
    const auto& compiler_invocation = compiler_instance.getInvocation();
    auto& compiler_frontend_ops = compiler_invocation.getFrontendOpts();
    //assert(compiler_frontend_ops == frontend_options); 
    
    
    auto llvm = std::make_unique<llvm::LLVMContext>();
    auto compile_action = std::make_unique<clang::EmitLLVMOnlyAction>(llvm.get());
    
    if (compiler_instance.ExecuteAction(*compile_action)) 
    {
        printf("tried compiling\n");
        if(compiler_instance.getDiagnostics().getNumErrors() == 0)
        {
            std::cout << "compilation worked\n";
            
            
            std::unique_ptr<llvm::Module> module = compile_action->takeModule();
            
            if(module)
            {
                printf("module loaded\n");
                
                Clang_Context* clang_ctx = (Clang_Context*) clang_ctx_ptr;
                
                //Optimizations
                clang_ctx->module_pass_manager.run(*module, clang_ctx->module_analysis_manager);
                
                //create JIT
                llvm::EngineBuilder builder(std::move(module));
                builder.setMCJITMemoryManager(std::make_unique<llvm::SectionMemoryManager>());
                builder.setOptLevel(llvm::CodeGenOpt::Level::Aggressive);
                
                llvm::ExecutionEngine *engine = builder.create();
                
                
                if (engine) 
                {
                    engine->finalizeObject();
                    
                    auto audio_callback_f = (audio_callback_t)engine->getFunctionAddress("audio_callback_wrapper");
                    auto default_parameters_f = (default_parameters_t)engine->getFunctionAddress("default_parameters_wrapper");
                    auto initialize_state_f = (initialize_state_t)engine->getFunctionAddress("initialize_state_wrapper");
                    
                    
                    assert(audio_callback_f && default_parameters_f && initialize_state_f);
                    return Plugin_Handle{
                        true, 
                        (void*)engine, 
                        audio_callback_f, 
                        default_parameters_f, 
                        initialize_state_f,
                        descriptor
                    };
                    
                }
                
            }
            
        }
    }
    
    
    
    Plugin_Handle handle;
    
    return {false};
}


bool parse_plugin_descriptor(const clang::CXXRecordDecl* parameters_struct_decl, 
                             const clang::CXXRecordDecl* state_struct_decl,
                             Plugin_Descriptor& plugin_descriptor)
{
    if(parameters_struct_decl->isPolymorphic() || 
       state_struct_decl->isPolymorphic())
    {
        std::cout<<"Cannot be polymorphic\n";
        return false;
    }
    
    const clang::ASTRecordLayout& parameters_struct_layout  = parameters_struct_decl->getASTContext().getASTRecordLayout(parameters_struct_decl);
    
    auto parameters_struct_size = parameters_struct_layout.getSize();
    auto parameters_struct_alignment = parameters_struct_layout.getAlignment();
    
    
    const clang::ASTRecordLayout& state_struct_layout  = state_struct_decl->getASTContext().getASTRecordLayout(state_struct_decl);
    
    auto state_struct_size = state_struct_layout.getSize();
    auto state_struct_alignment = state_struct_layout.getAlignment();
    
    
    
    //plugin_descriptor.name = allocate_and_copy_llvm_stringref(plugin_descriptor_declaration->getName());
    plugin_descriptor.parameters_struct.size = parameters_struct_size.getQuantity();
    plugin_descriptor.parameters_struct.alignment = parameters_struct_alignment.getQuantity();
    
    plugin_descriptor.state_struct.size = state_struct_size.getQuantity();
    plugin_descriptor.state_struct.alignment = state_struct_alignment.getQuantity();
    
    
    u32 num_fields = 0;
    
    for(const auto* _ : parameters_struct_decl->fields())
    {
        num_fields++;
    }
    
    auto *valid_parameters = (Plugin_Descriptor_Parameter*)malloc(sizeof(Plugin_Descriptor_Parameter) * num_fields);
    
    u32 valid_parameter_idx = 0;
    for(const auto* field : parameters_struct_decl->fields())
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
            plugin_parameter.offset = parameters_struct_layout.getFieldOffset(index) / 8;
            
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

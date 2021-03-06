#include "llvm_include.h"


#if _WIN32
#pragma comment(lib, "version.lib")

/* date = November 15th 2021 1:55 pm */
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iterator>
#include "math.h"


#include "windows.h"

#include "base.h"
#include "structs.h"
#include "memory.h"
#include "audio.h"
#include "dsp.h"
#include "plugin.h"
#include "compiler.h"

struct LLVM_Context {
    llvm::LLVMContext llvm_context;
    std::unique_ptr<llvm::Module> wrapper_module;
};

struct Decl_Handle{
    Custom_Error error;
    union {
        const clang::FunctionDecl* fun;
        const clang::CXXRecordDecl* record;
    };
};

struct Plugin_Required_Decls{
    bool worked;
    Decl_Handle audio_callback;
    Decl_Handle default_parameters;
    Decl_Handle initialize_state;
    Decl_Handle parameters_struct;
    Decl_Handle state_struct;
};


struct Plugin_Clang_Context{
    Plugin_Required_Decls decls;
    llvm::ExecutionEngine *jit_engine;
    clang::ASTContext *ast_context;
    clang::CompilerInstance *compiler_instance;
};



LLVM_Context* create_llvm_context_impl();

internal String allocate_and_copy_llvm_stringref(Arena *allocator, llvm::StringRef llvm_stringref)
{
    String new_string;
    new_string.size = llvm_stringref.size();
    new_string.str = (char *)arena_allocate(allocator, sizeof(char) * llvm_stringref.size()); 
    strncpy(new_string.str, llvm_stringref.data(), new_string.size);
    return new_string;
}


internal String allocate_and_copy_std_string(Arena *allocator, const std::string& std_string)
{
    String new_string;
    new_string.size = std_string.size();
    new_string.str = (char *)arena_allocate(allocator, sizeof(char) * std_string.size()); 
    strncpy(new_string.str, std_string.data(), new_string.size);
    return new_string;
}

template<typename Exec>
class Consumer : public clang::ASTConsumer
{
    public:
    explicit Consumer(Exec&& exec) : exec(exec), clang::ASTConsumer() { }
    
    virtual void HandleTranslationUnit(clang::ASTContext &Context)
    {
        if(Context.getDiagnostics().hasErrorOccurred()) return;
        exec(Context);
    }
    
    Exec& exec;
};



template<typename Exec>
class Action : public clang::ASTFrontendAction {
    
    Exec& exec;
    
    public:
    
    explicit Action(Exec&& exec) : exec{exec} {}
    
    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
        return std::make_unique<Consumer<Exec>>(exec);
    }
};


template<typename Exec>
internal Action<Exec> make_action(Exec&&  exec)
{
    return Action<Exec>(exec); 
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
    
    Exec& exec;
};


template<typename Exec>
internal MatchCallback<Exec> make_match_callback(Exec&&  exec)
{
    return MatchCallback<Exec>(exec); 
}

template<typename Matcher, typename Exec>
internal void match_ast(Matcher& matcher, Exec& exec, clang::ASTContext& ast_ctx)
{
    using namespace clang::ast_matchers;
    
    auto match_finder = MatchFinder{};
    
    
    auto matcher_callback = make_match_callback(exec);
    match_finder.addMatcher(matcher, &matcher_callback);
    match_finder.matchAST(ast_ctx);
} 

template<typename Matcher, typename Exec, typename Node>
internal void match_node(Matcher& matcher, Exec& exec, const Node& node, clang::ASTContext& ast_ctx)
{
    using namespace clang::ast_matchers;
    
    auto match_finder = MatchFinder{};
    
    auto matcher_callback = make_match_callback(exec);
    match_finder.addMatcher(matcher, &matcher_callback);
    match_finder.match<Node>(node, ast_ctx);
} 



Compiler_Location fun_to_return_loc(const clang::FunctionDecl& fun, const clang::SourceManager& source_manager)
{
    auto location = fun.getReturnTypeSourceRange().getBegin();
    auto [file_id, offset] = source_manager.getDecomposedLoc(location);
    return{
        .line = source_manager.getLineNumber(file_id, offset),
        .column = source_manager.getColumnNumber(file_id, offset)
    };
};


Compiler_Location decl_to_loc(const clang::Decl& decl, const clang::SourceManager& source_manager)
{
    
    clang::SourceLocation location = decl.getLocation();
    auto [file_id, offset] = source_manager.getDecomposedLoc(location);
    return{
        .line = source_manager.getLineNumber(file_id, offset),
        .column = source_manager.getColumnNumber(file_id, offset)
    };
};

internal void print_parameter(Plugin_Descriptor_Parameter parameter)
{
    switch(parameter.type){
        case Int : {
            std::cout << "Int type : min = " << parameter.int_param.min << ", max = " << parameter.int_param.max << "\n\n";
            
        }break;
        
        case Float : {
            std::cout << "Float type : min = " << parameter.float_param.min << ", max = " << parameter.float_param.max << "\n\n";
            
        }break;
        
        case Enum : {
            std::cout << "Enum type :\n";
            
            auto enum_param = parameter.enum_param;
            for(auto i = 0; i < enum_param.num_entries; i++)
            {
                //TODO si cest pas NULL-terminated ??a va faire une grosse erreur
                std::cout << "   " << enum_param.entries[i].name.str << " : " << enum_param.entries[i].value << "\n";
            }
            std::cout << "\n";
        }break;
    }
}



Plugin_Required_Decls find_decls(clang::ASTContext& ast_ctx);

Plugin_Descriptor parse_plugin_descriptor(const clang::CXXRecordDecl* parameters_struct_decl, 
                                          const clang::CXXRecordDecl* state_struct_decl,
                                          const clang::SourceManager& source_manager,
                                          Arena *allocator);

std::unique_ptr<llvm::MemoryBuffer> 
rewrite_plugin_source(Plugin_Required_Decls decls,
                      clang::SourceManager& source_manager,
                      clang::LangOptions& language_options,
                      clang::FileID plugin_source_file_id);

void jit_compile(llvm::MemoryBufferRef new_buffer, clang::CompilerInstance& compiler_instance,
                 Plugin_Descriptor& descriptor,
                 Plugin_Required_Decls& decls,
                 llvm::LLVMContext *llvm_context,
                 llvm::Module *wrapper_module,
                 clang::ASTContext *ast_context,
                 Plugin *plugin,
                 Arena *allocator);

Plugin try_compile_impl(const char* filename, 
                        LLVM_Context* clang_cts,
                        Arena *allocator,
                        Plugin_Clang_Context *previous_clang_ctx);


#ifdef DEBUG
extern "C" __declspec(dllexport)
#endif
void* create_llvm_context()
{
    return (void*) create_llvm_context_impl();
}



double oct_sin_64(double d1) { return sin(d1); }
double oct_cos_64(double d1) { return cos(d1); }
double oct_tan_64(double d1) { return tan(d1); }
double oct_fabs_64(double d1) { return fabs(d1); }
double oct_pow_64(double d1, double d2) { return pow(d1, d2); }
double oct_fmod_64(double d1, double d2) { return fmod(d1, d2); }
double oct_ceil_64(double d1) { return ceil(d1); }
double oct_floor_64(double d1) { return floor(d1); }
double oct_sqrt_64(double d1) { return sqrt(d1); }
double oct_exp_64(double d1) { return exp(d1); }
double oct_log10_64(double d1) { return log10(d1); }
double oct_log_64(double d1) { return log(d1); }
double oct_asin_64(double d1) { return asin(d1); }
double oct_acos_64(double d1) { return acos(d1); }
double oct_atan_64(double d1) { return atan(d1); }
double oct_atan2_64(double d1, double d2) { return atan2(d1, d2); }
double oct_sinh_64(double d1) { return sinh(d1); }
double oct_cosh_64(double d1) { return cosh(d1); }
double oct_tanh_64(double d1) { return tanh(d1); }


float oct_sin_32(float d1) { return sin(d1); }
float oct_cos_32(float d1) { return cos(d1); }
float oct_tan_32(float d1) { return tan(d1); }
float oct_fabs_32(float d1) { return fabs(d1); }
float oct_pow_32(float d1, float d2) { return pow(d1, d2); }
float oct_fmod_32(float d1, float d2) { return fmod(d1, d2); }
float oct_ceil_32(float d1) { return ceil(d1); }
float oct_floor_32(float d1) { return floor(d1); }
float oct_sqrt_32(float d1) { return sqrt(d1); }
float oct_exp_32(float d1) { return exp(d1); }
float oct_log10_32(float d1) { return log10(d1); }
float oct_log_32(float d1) { return log(d1); }
float oct_asin_32(float d1) { return asin(d1); }
float oct_acos_32(float d1) { return acos(d1); }
float oct_atan_32(float d1) { return atan(d1); }
float oct_atan2_32(float d1, float d2) { return atan2(d1, d2); }
float oct_sinh_32(float d1) { return sinh(d1); }
float oct_cosh_32(float d1) { return cosh(d1); }
float oct_tanh_32(float d1) { return tanh(d1); }

IPP_FFT_Context *plugin_fft_initialize(Initializer *initializer)
{
    return initializer->ipp_context;
}

LLVM_Context* create_llvm_context_impl()
{
    //magic stuff
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    
    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
    
    llvm::sys::DynamicLibrary::AddSymbol("sin_32", oct_sin_32);
    llvm::sys::DynamicLibrary::AddSymbol("cos_32", oct_cos_32);
    llvm::sys::DynamicLibrary::AddSymbol("tan_32", oct_tan_32);
    llvm::sys::DynamicLibrary::AddSymbol("fabs_32", oct_fabs_32);
    llvm::sys::DynamicLibrary::AddSymbol("pow_32", oct_pow_32);
    llvm::sys::DynamicLibrary::AddSymbol("fmod_32", oct_fmod_32);
    llvm::sys::DynamicLibrary::AddSymbol("ceil_32", oct_ceil_32);
    llvm::sys::DynamicLibrary::AddSymbol("floor_32", oct_floor_32);
    llvm::sys::DynamicLibrary::AddSymbol("sqrt_32", oct_sqrt_32);
    llvm::sys::DynamicLibrary::AddSymbol("exp_32", oct_exp_32);
    llvm::sys::DynamicLibrary::AddSymbol("log10_32", oct_log10_32);
    llvm::sys::DynamicLibrary::AddSymbol("log_32", oct_log_32);
    llvm::sys::DynamicLibrary::AddSymbol("asin_32", oct_asin_32);
    llvm::sys::DynamicLibrary::AddSymbol("acos_32", oct_acos_32);
    llvm::sys::DynamicLibrary::AddSymbol("atan_32", oct_atan_32);
    llvm::sys::DynamicLibrary::AddSymbol("atan2_32", oct_atan2_32);
    llvm::sys::DynamicLibrary::AddSymbol("sinh_32", oct_sinh_32);
    llvm::sys::DynamicLibrary::AddSymbol("cosh_32", oct_cosh_32);
    llvm::sys::DynamicLibrary::AddSymbol("tanh_32", oct_tanh_32);
    
    llvm::sys::DynamicLibrary::AddSymbol("sin_64", oct_sin_64);
    llvm::sys::DynamicLibrary::AddSymbol("cos_64", oct_cos_64);
    llvm::sys::DynamicLibrary::AddSymbol("tan_64", oct_tan_64);
    llvm::sys::DynamicLibrary::AddSymbol("fabs_64", oct_fabs_64);
    llvm::sys::DynamicLibrary::AddSymbol("pow_64", oct_pow_64);
    llvm::sys::DynamicLibrary::AddSymbol("fmod_64", oct_fmod_64);
    llvm::sys::DynamicLibrary::AddSymbol("ceil_64", oct_ceil_64);
    llvm::sys::DynamicLibrary::AddSymbol("floor_64", oct_floor_64);
    llvm::sys::DynamicLibrary::AddSymbol("sqrt_64", oct_sqrt_64);
    llvm::sys::DynamicLibrary::AddSymbol("exp_64", oct_exp_64);
    llvm::sys::DynamicLibrary::AddSymbol("log10_64", oct_log10_64);
    llvm::sys::DynamicLibrary::AddSymbol("log_64", oct_log_64);
    llvm::sys::DynamicLibrary::AddSymbol("asin_64", oct_asin_64);
    llvm::sys::DynamicLibrary::AddSymbol("acos_64", oct_acos_64);
    llvm::sys::DynamicLibrary::AddSymbol("atan_64", oct_atan_64);
    llvm::sys::DynamicLibrary::AddSymbol("atan2_64", oct_atan2_64);
    llvm::sys::DynamicLibrary::AddSymbol("sinh_64", oct_sinh_64);
    llvm::sys::DynamicLibrary::AddSymbol("cosh_64", oct_cosh_64);
    llvm::sys::DynamicLibrary::AddSymbol("tanh_64", oct_tanh_64);
    
    llvm::sys::DynamicLibrary::AddSymbol("plugin_allocate_buffer_app", plugin_allocate_buffer);
    llvm::sys::DynamicLibrary::AddSymbol("plugin_allocate_buffers_app", plugin_allocate_buffers);
    llvm::sys::DynamicLibrary::AddSymbol("plugin_allocate_bytes_app", plugin_allocate_bytes);
    
    llvm::sys::DynamicLibrary::AddSymbol("fft_initialize", plugin_fft_initialize);
    llvm::sys::DynamicLibrary::AddSymbol("fft_forward", fft_forward);
    llvm::sys::DynamicLibrary::AddSymbol("fft_reverse", fft_reverse);
    llvm::sys::DynamicLibrary::AddSymbol("pythagore_array", pythagore_array);
    llvm::sys::DynamicLibrary::AddSymbol("windowing_hamming", windowing_hamming);
    
    llvm::sys::DynamicLibrary::AddSymbol("phasor_32_array", phasor_32_array);
    llvm::sys::DynamicLibrary::AddSymbol("sin_32_array", sin_32_array);
    llvm::sys::DynamicLibrary::AddSymbol("triangle_32_array", triangle_32_array);
    
    llvm::sys::DynamicLibrary::AddSymbol("gain_32_array", gain_32_array);
    llvm::sys::DynamicLibrary::AddSymbol("dc_offset_32_array", dc_offset_32_array);
    llvm::sys::DynamicLibrary::AddSymbol("sqrt_32_array", sqrt_32_array);
    llvm::sys::DynamicLibrary::AddSymbol("abs_32_array", abs_32_array);
    llvm::sys::DynamicLibrary::AddSymbol("to_db_32_array", to_db_32_array);
    llvm::sys::DynamicLibrary::AddSymbol("from_db_32_array", from_db_32_array);
    llvm::sys::DynamicLibrary::AddSymbol("ln_32_array", ln_32_array);
    llvm::sys::DynamicLibrary::AddSymbol("log2_32_array", log2_32_array);
    llvm::sys::DynamicLibrary::AddSymbol("log10_32_array", log10_32_array);
    
    
    llvm::sys::DynamicLibrary::AddSymbol("gain_ip_32_array", gain_ip_32_array);
    llvm::sys::DynamicLibrary::AddSymbol("dc_offset_ip_32_array", dc_offset_ip_32_array);
    llvm::sys::DynamicLibrary::AddSymbol("sqrt_ip_32_array", sqrt_ip_32_array);
    llvm::sys::DynamicLibrary::AddSymbol("abs_ip_32_array", abs_ip_32_array);
    llvm::sys::DynamicLibrary::AddSymbol("ln_ip_32_array", ln_ip_32_array);
    llvm::sys::DynamicLibrary::AddSymbol("log2_ip_32_array", log2_ip_32_array);
    llvm::sys::DynamicLibrary::AddSymbol("log10_ip_32_array", log10_ip_32_array);
    
    llvm::sys::DynamicLibrary::AddSymbol("set_array", set_array);
    llvm::sys::DynamicLibrary::AddSymbol("copy_array", copy_array);
    llvm::sys::DynamicLibrary::AddSymbol("zero_array", zero_array);
    
    
    
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
    
    
    auto* llvm_ctx = new LLVM_Context();
    
    {
        auto compiler_instance = clang::CompilerInstance();
        
        clang::LangOptions language_options{};
        language_options.Bool = 1;
        language_options.CPlusPlus = 1;
        language_options.RTTI = 0;
        language_options.CXXExceptions = 0;
        language_options.MSVCCompat = 1;
        language_options.MicrosoftExt = 1;
        
        auto triple = llvm::sys::getDefaultTargetTriple();
        auto triple_str = "-triple=" + triple;
        
        compiler_instance.createDiagnostics();
        clang::CompilerInvocation::CreateFromArgs(compiler_instance.getInvocation(),
                                                  {
                                                      triple_str.c_str(),
                                                      "-xc++",
                                                      "-std=c++17",
                                                      "-fdeclspec",
                                                      "-fms-extensions",
                                                      "-fms-compatibility",
                                                      "-fcxx-exceptions", 
                                                      "-fexceptions",
                                                      "-v",
                                                  }, 
                                                  compiler_instance.getDiagnostics());
        
        compiler_instance.getTargetOpts().Triple = triple;
        compiler_instance.getLangOpts() = language_options;
        auto& header_opts = compiler_instance.getHeaderSearchOpts();
        /*
        Clang.getHeaderSearchOpts().ResourceDir =
            CompilerInvocation::GetResourcesPath(argv[0], MainAddr);
        */
        //TODO delete this ?
        compiler_instance.createTarget();
        
        auto kind = clang::InputKind(clang::Language::CXX);
        clang::FrontendInputFile input_file = clang::FrontendInputFile("../wrapper_plugin_object.cpp", kind);
        
        compiler_instance.getFrontendOpts().Inputs.clear();
        compiler_instance.getFrontendOpts().Inputs.push_back(std::move(input_file));
        
        auto back = compiler_instance.getFrontendOpts().Inputs.back();
        
        
        auto compile_action = clang::EmitLLVMOnlyAction(&llvm_ctx->llvm_context);
        (compiler_instance.ExecuteAction(compile_action));
        llvm_ctx->wrapper_module = compile_action.takeModule();
    }
    
    return llvm_ctx;
}

#ifdef DEBUG
extern "C" __declspec(dllexport)
#endif
Plugin try_compile(const char* filename, void* llvm_ctx_ptr, Arena *allocator, void *previous_clang_ctx)
{
    return try_compile_impl(filename, (LLVM_Context*) llvm_ctx_ptr, allocator, (Plugin_Clang_Context*) previous_clang_ctx);
}

Clang_Error to_clang_error(const std::pair< clang::SourceLocation, std::string > &error, const clang::SourceManager &source_manager,
                           Arena *allocator)
{
    auto [file_id, offset] = source_manager.getDecomposedLoc(error.first);
    Compiler_Location loc = {
        .line = source_manager.getLineNumber(file_id, offset),
        .column = source_manager.getColumnNumber(file_id, offset)
    };
    
    return { allocate_and_copy_std_string(allocator, error.second), loc};
}

void errors_push_clang(Clang_Error_Log *error_log, Clang_Error new_error)
{
    if(error_log->count >= error_log->capacity) return; 
    error_log->errors[error_log->count++] = new_error;
}


Plugin try_compile_impl(const char* filename, LLVM_Context* llvm_ctx, Arena *allocator, Plugin_Clang_Context *previous_clang_ctx)
{
    Plugin plugin = {};
    auto *compiler_instance = new clang::CompilerInstance{};
    clang::TextDiagnosticBuffer diagnostics = {};
    
    {
        clang::LangOptions language_options{};
        language_options.Bool = 1;
        language_options.CPlusPlus = 1;
        language_options.RTTI = 0;
        language_options.CXXExceptions = 0;
        language_options.MSVCCompat = 1;
        language_options.MicrosoftExt = 1;
        
        compiler_instance->createDiagnostics(&diagnostics, false);
        
        auto triple = llvm::sys::getDefaultTargetTriple();
        auto triple_str = "-triple=" + triple;
        
        clang::CompilerInvocation::CreateFromArgs(compiler_instance->getInvocation(),
                                                  {
                                                      triple_str.c_str(),
                                                      "-Ofast", 
                                                      "-fms-extensions", 
                                                      "-ffast-math", 
                                                      "-fdenormal-fp-math=positive-zero"
                                                  }, 
                                                  compiler_instance->getDiagnostics());
        
        compiler_instance->getTargetOpts().Triple = triple;
        compiler_instance->getLangOpts() = language_options;
        auto& header_opts = compiler_instance->getHeaderSearchOpts();
        header_opts.AddPath(".", clang::frontend::Quoted , false, false);
        header_opts.UseBuiltinIncludes = 0; //TODO ????
        
        //TODO est-ce que je peux enlever cette ligne ? 
        compiler_instance->createTarget();
    }
    
    String plugin_filename;
    {
        auto kind = clang::InputKind(clang::Language::CXX);
        clang::FrontendInputFile input_file = clang::FrontendInputFile(filename, kind);
        
        auto split_input = input_file.getFile().rsplit('\\');
        if(split_input.second.size() == 0)
            plugin_filename =  allocate_and_copy_llvm_stringref(allocator, split_input.first);
        else
            plugin_filename =  allocate_and_copy_llvm_stringref(allocator, split_input.second);
        
        compiler_instance->getFrontendOpts().Inputs.push_back(std::move(input_file));
    }
    
    std::unique_ptr<llvm::MemoryBuffer> new_buffer = nullptr;
    Plugin_Required_Decls decls;
    
    clang::ASTContext *ast_context = nullptr;
    
    bool action_result; 
    {
        auto visit_ast = [&](clang::ASTContext& ast_ctx){};
        auto dummy_action = make_action(visit_ast);
        dummy_action.BeginSourceFile(*compiler_instance, compiler_instance->getFrontendOpts().Inputs.back());
        action_result = static_cast<bool>(dummy_action.Execute());
        ast_context = &compiler_instance->getASTContext();
        compiler_instance->resetAndLeakASTContext();
        dummy_action.EndSourceFile();
    }
    
    if(diagnostics.getNumErrors() != 0)
    {
        ensure(action_result == false);
        
        plugin.clang_error_log = {
            (Clang_Error*)arena_allocate(allocator, sizeof(Clang_Error) * diagnostics.getNumErrors()),
            0,
            diagnostics.getNumErrors()
        };
        
        for(auto error_it = diagnostics.err_begin(); error_it < diagnostics.err_end(); error_it++)
        {
            errors_push_clang(&plugin.clang_error_log, to_clang_error(*error_it, compiler_instance->getSourceManager(), allocator));
        }
        
        plugin.failure_stage = Compiler_Failure_Stage_Clang_First_Pass;
        ast_context->Release();
        delete compiler_instance;
        return plugin;
    }
    
    
    decls = find_decls(*ast_context);
    if(!decls.worked)
    {
        
        plugin.failure_stage = Compiler_Failure_Stage_Finding_Decls;
        plugin.decls_search_log = {
            decls.audio_callback.error,
            decls.default_parameters.error,
            decls.initialize_state.error,
            decls.parameters_struct.error,
            decls.state_struct.error
        };
        
        ast_context->Release();
        delete compiler_instance;
        return plugin;
    }
    
    bool plugin_needs_to_be_reinitialized = true;
    if(previous_clang_ctx != nullptr)
    {
        ensure(previous_clang_ctx->ast_context != nullptr);
        
        //parameter struct
        {
        }
    }
    
    plugin.descriptor = parse_plugin_descriptor(decls.parameters_struct.record, 
                                                decls.state_struct.record, 
                                                compiler_instance->getSourceManager(),
                                                allocator);
    plugin.descriptor.name = plugin_filename;
    
    if(plugin.descriptor.error.flag != Compiler_Success){
        plugin.failure_stage = Compiler_Failure_Stage_Parsing_Parameters; 
        
        ast_context->Release();
        delete compiler_instance;
        return plugin;
    }
    
    new_buffer = rewrite_plugin_source(decls, 
                                       compiler_instance->getSourceManager(), 
                                       compiler_instance->getLangOpts(), 
                                       compiler_instance->getSourceManager().getMainFileID());
    ensure(new_buffer);
    
    diagnostics.clear();
    jit_compile(new_buffer->getMemBufferRef(), 
                *compiler_instance, 
                plugin.descriptor,
                decls,
                &llvm_ctx->llvm_context,
                llvm_ctx->wrapper_module.get(),
                ast_context,
                &plugin, allocator);
    if(plugin.failure_stage == Compiler_Failure_Stage_Clang_Second_Pass){
        ast_context->Release();
        delete compiler_instance;
    }
    else {
        ensure(plugin.failure_stage == Compiler_Failure_Stage_No_Failure);
    }
    return plugin;
}

Decl_Handle find_audio_callback(clang::ASTContext& ast_ctx)
{
    auto& source_manager = ast_ctx.getSourceManager();
    Custom_Error error = { Compiler_Success };
    using namespace clang::ast_matchers;
    
    
    const clang::FunctionDecl* audio_callback_decl = nullptr;
    
    auto audio_callback_matcher = functionDecl(hasName("audio_callback")).bind("callback");
    auto audio_callback_lambda = [&](const auto& result)
    {
        if(error.flag != Compiler_Success) return; 
        
        const clang::FunctionDecl* decl = result.Nodes.getNodeAs<clang::FunctionDecl>("callback");
        
        if(decl == nullptr) return;
        if(!decl->isThisDeclarationADefinition()) return;
        
        if(audio_callback_decl == nullptr)
            audio_callback_decl = decl;
        else
            error = {Compiler_Too_Many_Fun, decl_to_loc(*audio_callback_decl, source_manager), decl_to_loc(*decl, source_manager)}; 
    };
    match_ast(audio_callback_matcher, audio_callback_lambda, ast_ctx);
    
    if(audio_callback_decl == nullptr)
        error = { Compiler_No_Fun }; 
    
    if(error.flag != Compiler_Success) return Decl_Handle { error };
    
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
    
    auto audio_callback_has_right_signature_lambda = [&](const auto& result) {
        const clang::FunctionDecl* decl = result.Nodes.getNodeAs<clang::FunctionDecl>("callback");
        if(decl == nullptr) return;
        if(!decl->isThisDeclarationADefinition()) return;
        has_audio_callback_the_right_signature = true;
    };
    
    match_node(audio_callback_has_right_signature_matcher,
               audio_callback_has_right_signature_lambda,
               *audio_callback_decl,
               ast_ctx);
    
    if(has_audio_callback_the_right_signature == false){
        error = { Compiler_Wrong_Signature_Fun, decl_to_loc(*audio_callback_decl, source_manager)};
    }
    
    return Decl_Handle{
        .error = error,
        .fun = audio_callback_decl
    };
}


Decl_Handle find_default_parameters(clang::ASTContext& ast_ctx)
{
    auto& source_manager = ast_ctx.getSourceManager();
    Custom_Error error = { Compiler_Success };
    using namespace clang::ast_matchers;
    
    
    const clang::FunctionDecl* default_parameters_decl = nullptr;
    
    auto default_parameters_matcher = functionDecl(hasName("default_parameters")).bind("default_param");
    
    auto default_parameters_lambda = [&](const auto& result) {
        if(error.flag != Compiler_Success) return; 
        
        const clang::FunctionDecl* decl = result.Nodes.getNodeAs<clang::FunctionDecl>("default_param");
        
        if(decl == nullptr) return;
        if(!decl->isThisDeclarationADefinition()) return;
        if(default_parameters_decl == nullptr)
        {
            default_parameters_decl = decl;
        }
        else
        {
            error = { Compiler_Too_Many_Fun, decl_to_loc(*decl, source_manager)};
        }
    };
    match_ast(default_parameters_matcher, default_parameters_lambda, ast_ctx);
    
    if(default_parameters_decl == nullptr)
        error = {Compiler_No_Fun};
    
    if(error.flag != Compiler_Success) return Decl_Handle { error };
    
    if(default_parameters_decl->getNumParams() != 0)
        error = { Compiler_Wrong_Signature_Fun, decl_to_loc(*default_parameters_decl, source_manager)};
    
    return Decl_Handle{
        .error = error,
        .fun = default_parameters_decl
    };
}


Decl_Handle find_initialize_state(clang::ASTContext& ast_ctx)
{
    auto& source_manager = ast_ctx.getSourceManager();
    Custom_Error error = { Compiler_Success };
    using namespace clang::ast_matchers;
    
    const clang::FunctionDecl* initialize_state_decl = nullptr;
    
    auto initialize_state_matcher = functionDecl(hasName("initialize_state")).bind("initialize_state");
    auto initialize_state_lambda = [&](const auto& result)
    {
        if(error.flag != Compiler_Success) return; 
        
        const clang::FunctionDecl* decl = result.Nodes.getNodeAs<clang::FunctionDecl>("initialize_state");
        
        if(decl == nullptr) return;
        if(!decl->isThisDeclarationADefinition()) return;
        if(initialize_state_decl == nullptr)
        {
            initialize_state_decl = decl;
        }
        else
        {
            error = { Compiler_Too_Many_Fun, decl_to_loc(*decl, source_manager)};
        }
    };
    match_ast(initialize_state_matcher, initialize_state_lambda, ast_ctx);
    
    if(initialize_state_decl == nullptr)
        error = { Compiler_No_Fun };
    
    if(error.flag != Compiler_Success) return Decl_Handle { error };
    
    //TODO see Compiler_Error comment
    bool initializer_has_the_right_signature = [&](){
        
        auto num_params = initialize_state_decl->getNumParams();
        if(num_params != 4)
        {
            std::cout << "invalid parameter count\n";
            return false;
        }
        
        //TODO check que c'est ni const ni volatile
        const auto& parameters_type = *initialize_state_decl->getParamDecl(0)->getType().getCanonicalType();
        const auto& num_channels_type = *initialize_state_decl->getParamDecl(1)->getType().getCanonicalType();
        const auto& sample_rate_type = *initialize_state_decl->getParamDecl(2)->getType().getCanonicalType();
        const auto& allocator_type = *initialize_state_decl->getParamDecl(3)->getType();
        
        if(!num_channels_type.isSpecificBuiltinType(clang::BuiltinType::UInt))
        {
            std::cout << "second parameter should be unsigned int num_channels\n";
            return false;
        }
        
        //TODO ensure que float est 32 bit
        if(!sample_rate_type.isSpecificBuiltinType(clang::BuiltinType::Float))
        {
            std::cout << "third parameter should be float sample_rate\n";
            return false;
        }
        
        if(!allocator_type.isVoidPointerType())
        {
            std::cout << "fourth parameter should be void *allocator\n"; 
            return false;
        }
        return true;
    }();
    
    if(!initializer_has_the_right_signature)
    {
        error = { Compiler_Wrong_Signature_Fun, decl_to_loc(*initialize_state_decl, source_manager)};
    }
    
    return Decl_Handle{
        .error = error, 
        .fun = initialize_state_decl
    };
}

Plugin_Required_Decls find_decls(clang::ASTContext& ast_ctx)
{
    auto& source_manager = ast_ctx.getSourceManager();
    Decl_Handle audio_callback_decl = find_audio_callback(ast_ctx);
    Decl_Handle default_parameters_decl = find_default_parameters(ast_ctx);
    Decl_Handle initialize_state_decl = find_initialize_state(ast_ctx);
    
    if(audio_callback_decl.error.flag != Compiler_Success ||
       default_parameters_decl.error.flag != Compiler_Success ||
       initialize_state_decl.error.flag != Compiler_Success)
    {
        return Plugin_Required_Decls{
            false,
            audio_callback_decl,
            default_parameters_decl,
            initialize_state_decl,
        };
    }
    
    const auto& default_parameters_return_type = 
        *default_parameters_decl.fun->getReturnType();
    
    const auto& initialize_state_return_type = 
        *initialize_state_decl.fun->getReturnType();
    
    const auto& initialize_state_parameter_type = *initialize_state_decl.fun->getParamDecl(0)->getType()->getPointeeType();
    
    const auto& audio_callback_parameter_type = *audio_callback_decl.fun->getParamDecl(0)->getType()->getPointeeType();
    
    const auto& audio_callback_state_type = *audio_callback_decl.fun->getParamDecl(1)->getType()->getPointeeType();
    
    const auto& parameters_type = default_parameters_return_type;
    const auto& state_type = initialize_state_return_type;
    
    /////////////////////////////////////////////////////////////
    Decl_Handle parameters_struct_decl = {Compiler_Success};
    
    if(&default_parameters_return_type != &initialize_state_parameter_type ||
       &default_parameters_return_type != &audio_callback_parameter_type)
    {
        parameters_struct_decl.error = { 
            Compiler_Types_Mismatch, 
            fun_to_return_loc(*default_parameters_decl.fun, source_manager),
            decl_to_loc(*initialize_state_decl.fun->getParamDecl(0), source_manager),
            decl_to_loc(*audio_callback_decl.fun->getParamDecl(0), source_manager)
        };
    }
    else if(!parameters_type.isRecordType())
    {
        parameters_struct_decl.error = { Compiler_Not_Record_Type, fun_to_return_loc(*default_parameters_decl.fun, source_manager)};
    }
    else 
    {
        parameters_struct_decl.record = parameters_type.getAsCXXRecordDecl();
        if(parameters_struct_decl.record->isPolymorphic())
        {
            parameters_struct_decl.error = { Compiler_Polymorphic, decl_to_loc(*parameters_struct_decl.record, source_manager)};
        }
    }
    
    /////////////////////////////////////////////////////////////
    Decl_Handle state_struct_decl = {Compiler_Success};
    
    if(&initialize_state_return_type != &audio_callback_state_type)
    {
        state_struct_decl.error = { 
            Compiler_Types_Mismatch, 
            decl_to_loc(*state_struct_decl.record, source_manager), fun_to_return_loc(*initialize_state_decl.fun, source_manager)
        };
    }
    else if(!state_type.isRecordType())
    {
        state_struct_decl.error = {
            Compiler_Not_Record_Type, 
            decl_to_loc(*state_struct_decl.record, source_manager)
        };
    }
    else 
    {
        state_struct_decl.record = state_type.getAsCXXRecordDecl();
        if(state_struct_decl.record->isPolymorphic())
        {
            state_struct_decl.error = {
                Compiler_Polymorphic,
                decl_to_loc(*state_struct_decl.record, source_manager)
            };
        }
    }
    
    bool worked = 
        parameters_struct_decl.error.flag == Compiler_Success && 
        state_struct_decl.error.flag == Compiler_Success;
    
    return {
        worked,
        audio_callback_decl,
        default_parameters_decl,
        initialize_state_decl,
        parameters_struct_decl,
        state_struct_decl
    };
}


Plugin_Descriptor parse_plugin_descriptor(const clang::CXXRecordDecl* parameters_struct_decl, 
                                          const clang::CXXRecordDecl* state_struct_decl,
                                          const clang::SourceManager& source_manager,
                                          Arena *allocator)
{
    const clang::ASTRecordLayout& parameters_struct_layout  = parameters_struct_decl->getASTContext().getASTRecordLayout(parameters_struct_decl);
    
    auto parameters_struct_size = parameters_struct_layout.getSize();
    auto parameters_struct_alignment = parameters_struct_layout.getAlignment();
    
    const clang::ASTRecordLayout& state_struct_layout  = state_struct_decl->getASTContext().getASTRecordLayout(state_struct_decl);
    
    auto state_struct_size = state_struct_layout.getSize();
    auto state_struct_alignment = state_struct_layout.getAlignment();
    
    
    //TODO c'est une liste I guess ?
    u32 num_annotated_parameters = 0;
    for(const auto* field : parameters_struct_decl->fields())
    {
        if (field->hasAttr<clang::AnnotateAttr>()) 
            num_annotated_parameters++;
    }
    
    if(num_annotated_parameters == 0)
    {
        return {
            .error = { Compiler_Success },
            .parameters_struct = {
                .size = parameters_struct_size.getQuantity(),
                .alignment = parameters_struct_alignment.getQuantity()
            },
            .state_struct = {
                .size = state_struct_size.getQuantity(),
                .alignment = state_struct_alignment.getQuantity()
            },
            .parameters = nullptr,
            .num_parameters = 0
        };
    }
    
    bool there_was_an_error = false;
    auto *anotated_parameters = (Plugin_Descriptor_Parameter*)arena_allocate(allocator, sizeof(Plugin_Descriptor_Parameter) * num_annotated_parameters);
    u32 anotated_parameter_idx = 0;
    for(const auto* field : parameters_struct_decl->fields())
    {
        if (field->hasAttr<clang::AnnotateAttr>()) {
            
            clang::AnnotateAttr* attr = field->getAttr<clang::AnnotateAttr>();
            
            std::vector<std::string> param_strings{};
            {
                std::istringstream iss(attr->getAnnotation().str());
                std::string token;
                while(std::getline(iss, token, ' ')) {
                    param_strings.push_back(token);
                }
            }
            
            Plugin_Descriptor_Parameter plugin_parameter_rename = [&param_strings, 
                                                                   &parameters_struct_layout,
                                                                   &field,
                                                                   &source_manager,
                                                                   &allocator] 
            {
                Plugin_Descriptor_Parameter parameter = { .error { .flag = Compiler_Success}};
                
                const auto& type = *field->getType();
                auto index = field->getFieldIndex();
                parameter.offset = parameters_struct_layout.getFieldOffset(index) / 8;
                parameter.name = allocate_and_copy_llvm_stringref(allocator, field->getName());
                
                if(param_strings.size() == 0)
                {
                    parameter.error = { Compiler_Empty_Annotation, decl_to_loc(*field, source_manager)};
                    return parameter;
                }
                else if (param_strings[0] == "Int") 
                {
                    parameter.type = Int;
                    const auto *maybe_int = llvm::dyn_cast<clang::BuiltinType>(&type);
                    if(maybe_int == nullptr || maybe_int->getKind() != clang::BuiltinType::Int)
                    {
                        parameter.error = { Compiler_Annotation_Type_Mismatch, decl_to_loc(*field, source_manager)};
                        return parameter;
                    }
                    
                    if(param_strings.size() != 3 ) 
                    {
                        parameter.error = { Compiler_Missing_Min_Max, decl_to_loc(*field, source_manager)};
                        return parameter;
                    }
                    char* min_end_ptr = nullptr;
                    i32 min = strtol(param_strings[1].c_str() , &min_end_ptr, 0);
                    if(min_end_ptr == nullptr)
                    {
                        parameter.error = { Compiler_Invalid_Min_Value, decl_to_loc(*field, source_manager)};
                    }
                    else 
                    {
                        parameter.int_param.min = min;
                    }
                    
                    char* max_end_ptr = nullptr;
                    i32 max = strtol(param_strings[2].c_str(), &max_end_ptr, 0);
                    if(max_end_ptr == nullptr)
                    {
                        parameter.error = { Compiler_Invalid_Max_Value, decl_to_loc(*field, source_manager)};
                    }
                    else {
                        parameter.int_param.max = max; //TODO conversion;
                    }
                    
                    if(min_end_ptr && max_end_ptr && min >= max)
                    {
                        parameter.error = { Compiler_Min_Greater_Than_Max, decl_to_loc(*field, source_manager)};
                    }
                    
                }
                else if (param_strings[0] == "Float") 
                {
                    parameter.type = Float;
                    const auto *maybe_float = llvm::dyn_cast<clang::BuiltinType>(&type);
                    if(maybe_float == nullptr || maybe_float->getKind() != clang::BuiltinType::Float)
                    {
                        parameter.error = { Compiler_Annotation_Type_Mismatch, decl_to_loc(*field, source_manager)};
                        return parameter;
                    }
                    if(param_strings.size() != 3 && param_strings.size() != 4) 
                    {
                        parameter.error = { Compiler_Missing_Min_Max, decl_to_loc(*field, source_manager)};
                        return parameter;
                    }
                    char* min_end_ptr = nullptr;
                    real32 min = strtof(param_strings[1].c_str(), &min_end_ptr);
                    if(min_end_ptr == nullptr)
                        parameter.error = { Compiler_Invalid_Min_Value, decl_to_loc(*field, source_manager)};
                    else
                        parameter.float_param.min = min;
                    
                    char* max_end_ptr = nullptr;
                    real32 max = strtof(param_strings[2].c_str(), &max_end_ptr);
                    if(max_end_ptr == nullptr)
                        parameter.error = { Compiler_Invalid_Max_Value, decl_to_loc(*field, source_manager)};
                    else 
                        parameter.float_param.max = max; //TODO conversion;
                    
                    if(min_end_ptr && max_end_ptr && min >= max)
                    {
                        parameter.error = { Compiler_Min_Greater_Than_Max, decl_to_loc(*field, source_manager)};
                    }
                    
                    
                    if(param_strings.size() == 3)
                    {
                        parameter.float_param.log = false;
                    }
                    else
                    {
                        if(param_strings[3] == "log")
                            parameter.float_param.log = true;
                        else
                            parameter.error = { Compiler_Invalid_Annotation, decl_to_loc(*field, source_manager)};
                        
                    }
                }
                else if (param_strings[0] == "Enum") 
                {
                    parameter.type = Enum;
                    const auto *maybe_enum = llvm::dyn_cast<clang::EnumType>(&type);
                    if(maybe_enum == nullptr)
                    {
                        parameter.error = { Compiler_Annotation_Type_Mismatch, decl_to_loc(*field, source_manager)};
                        return parameter;
                    }
                    
                    Parameter_Enum enum_param = {};
                    
                    auto *enum_decl = maybe_enum->getDecl();
                    //TODO ensure que le getIntegerType soit un int normal
                    for(auto _: maybe_enum->getDecl()->enumerators())
                        enum_param.num_entries++;
                    
                    enum_param.entries = (Parameter_Enum_Entry*)arena_allocate(allocator, sizeof(Parameter_Enum_Entry) * enum_param.num_entries);
                    
                    auto i = 0;
                    for(const auto *field : maybe_enum->getDecl()->enumerators())
                    {
                        enum_param.entries[i].name = allocate_and_copy_llvm_stringref(allocator, field->getName());
                        enum_param.entries[i].value = field->getInitVal().getExtValue(); 
                        i++;
                    }
                    
                    parameter.enum_param = enum_param;
                }
                else {
                    parameter.error = { Compiler_Invalid_Annotation, decl_to_loc(*field, source_manager)};
                }
                return parameter;
            }();
            
            if(plugin_parameter_rename.error.flag != Compiler_Success)
                there_was_an_error = true;
            anotated_parameters[anotated_parameter_idx++] = plugin_parameter_rename;
        }
    }
    
    return {
        .error = there_was_an_error ? Custom_Error{ Compiler_Error_Recurse } : Custom_Error{ Compiler_Success },
        .parameters_struct = {
            .size = parameters_struct_size.getQuantity(),
            .alignment = parameters_struct_alignment.getQuantity()
        },
        .state_struct = {
            .size = state_struct_size.getQuantity(),
            .alignment = state_struct_alignment.getQuantity()
        },
        .parameters = anotated_parameters,
        .num_parameters = num_annotated_parameters
    };
}


std::unique_ptr<llvm::MemoryBuffer> 
rewrite_plugin_source(Plugin_Required_Decls decls,
                      clang::SourceManager& source_manager,
                      clang::LangOptions& language_options,
                      clang::FileID plugin_source_file_id)
{
    
    auto audio_callback_range = decls.audio_callback.fun->getSourceRange();
    auto default_parameters_range = decls.default_parameters.fun->getSourceRange();
    auto initialize_state_range = decls.initialize_state.fun->getSourceRange();
    
    auto parameters_name = decls.parameters_struct.record->getNameAsString();
    auto state_name = decls.state_struct.record->getNameAsString();
    
    std::string audio_callback_type_wrapper = "\n"
        "extern \"C\" void audio_callback_type_wrapper(void* param_ptr, void* state_ptr,float** out_buffer, unsigned int num_channels, unsigned int num_samples, float sample_rate)\n"
        "{\n"
        + parameters_name  +"* param  = (" + parameters_name + "*)param_ptr;\n"
        + state_name  +"* state  = (" + state_name + "*)state_ptr;\n"
        "audio_callback(*param, *state, out_buffer, num_channels, num_samples, sample_rate);\n"
        "}\n";
    
    std::string default_parameters_type_wrapper = "\n"
        "extern \"C\" void default_parameters_type_wrapper(void* out_parameters_ptr)\n"
        "{\n"
        + parameters_name + "* out_parameters = (" + parameters_name + "*)out_parameters_ptr;\n"
        "*out_parameters = default_parameters();\n"
        "}\n";
    
    std::string initialize_state_type_wrapper = "\n"
        "extern \"C\" void initialize_state_type_wrapper(void* parameters_ptr, void* out_initial_state_ptr, unsigned int num_channels, float sample_rate, void *allocator)\n"
        "{\n"
        + parameters_name + "* parameters = (" + parameters_name + "*)parameters_ptr;\n"
        
        + state_name + "* out_initial_state = (" + state_name + "*)out_initial_state_ptr;\n"
        "*out_initial_state = initialize_state(*parameters, num_channels, sample_rate, allocator);\n"
        "}\n";
    
    clang::Rewriter rewriter{source_manager, language_options};
    
    rewriter.InsertTextAfter(audio_callback_range.getEnd().getLocWithOffset(1), audio_callback_type_wrapper);
    rewriter.InsertTextAfter(default_parameters_range.getEnd().getLocWithOffset(1), default_parameters_type_wrapper);
    rewriter.InsertTextAfter(initialize_state_range.getEnd().getLocWithOffset(1),  initialize_state_type_wrapper);
    
    auto start_loc = source_manager.getLocForStartOfFile(plugin_source_file_id);
    auto end_loc = source_manager.getLocForEndOfFile(plugin_source_file_id);
    auto range = clang::SourceRange(start_loc, end_loc);
    
    auto new_text = rewriter.getRewrittenText(range);
    return llvm::MemoryBuffer::getMemBufferCopy(new_text); //TODO unnecessary copy ?
}


void jit_compile(llvm::MemoryBufferRef new_buffer, clang::CompilerInstance& compiler_instance,
                 Plugin_Descriptor& descriptor,
                 Plugin_Required_Decls& decls,
                 llvm::LLVMContext *llvm_context,
                 llvm::Module *wrapper_module,
                 clang::ASTContext *ast_context,
                 Plugin *plugin, Arena *allocator)
{
    
    auto* diagnostics = static_cast<clang::TextDiagnosticBuffer*>(&compiler_instance.getDiagnosticClient()); 
    ensure(diagnostics->getNumErrors() == 0);
    
    auto new_file = clang::FrontendInputFile{
        new_buffer, 
        clang::InputKind{clang::Language::CXX}
    };
    
    auto compile_action = clang::EmitLLVMOnlyAction(llvm_context);
    compile_action.BeginSourceFile(compiler_instance, new_file);
    bool action_result = static_cast<bool>(compile_action.Execute());
    compile_action.EndSourceFile();
    
    u32 error_count = diagnostics->getNumErrors();
    
    if(error_count != 0)
    {
        ensure(action_result == false);
        //NOTE only possible because we set this ourselves
        plugin->clang_error_log = {
            (Clang_Error*)arena_allocate(allocator, sizeof(Clang_Error) * error_count),
            0,
            error_count
        };
        
        for(auto error_it = diagnostics->err_begin(); error_it < diagnostics->err_end(); error_it++)
        {
            errors_push_clang(&plugin->clang_error_log, {allocate_and_copy_std_string(allocator, error_it->second)});
        }
        plugin->failure_stage = { Compiler_Failure_Stage_Clang_Second_Pass };
        return;
    }
    std::unique_ptr<llvm::Module> module = compile_action.takeModule();
    
    
    
    ensure(module); 
    {
        //Optimizations
        llvm::PassBuilder passBuilder;
        llvm::LoopAnalysisManager loopAnalysisManager;
        llvm::FunctionAnalysisManager functionAnalysisManager;
        llvm::CGSCCAnalysisManager cGSCCAnalysisManager;
        llvm::ModuleAnalysisManager moduleAnalysisManager;
        
        passBuilder.registerModuleAnalyses(moduleAnalysisManager);
        passBuilder.registerCGSCCAnalyses(cGSCCAnalysisManager);
        functionAnalysisManager.registerPass([&]{ return passBuilder.buildDefaultAAPipeline(); });
        passBuilder.registerFunctionAnalyses(functionAnalysisManager);
        passBuilder.registerLoopAnalyses(loopAnalysisManager);
        passBuilder.crossRegisterProxies(loopAnalysisManager, 
                                         functionAnalysisManager, 
                                         cGSCCAnalysisManager, 
                                         moduleAnalysisManager);
        
        llvm::ModulePassManager module_pass_manager = passBuilder.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);
        module_pass_manager.run(*module, moduleAnalysisManager);
    }
    
    llvm::ExecutionEngine *engine;
    {
        
        auto wrapper_clone = llvm::CloneModule(*wrapper_module);
        ensure(!llvm::Linker::linkModules(*module, std::move(wrapper_clone) ));
        llvm::EngineBuilder builder(std::move(module));
        
        builder.setMCJITMemoryManager(std::make_unique<llvm::SectionMemoryManager>());
        builder.setOptLevel(llvm::CodeGenOpt::Level::Aggressive);
        
        engine = builder.create();
        ensure(engine); 
        engine->finalizeObject();
    }
    
    auto audio_callback_f = (audio_callback_t)engine->getFunctionAddress("audio_callback_type_wrapper");
    auto default_parameters_f = (default_parameters_t)engine->getFunctionAddress("default_parameters_type_wrapper");
    auto initialize_state_f = (initialize_state_t)engine->getFunctionAddress("initialize_state_error_wrapper");
    
    
    ensure(audio_callback_f && default_parameters_f && initialize_state_f);
    
    plugin->failure_stage = { Compiler_Failure_Stage_No_Failure };
    plugin->descriptor = descriptor;
    
    Plugin_Clang_Context *plugin_clang_ctx = new Plugin_Clang_Context {
        decls,
        engine,
        ast_context,
        &compiler_instance
    };
    plugin->clang_ctx = (void*)plugin_clang_ctx; 
    plugin->audio_callback_f = audio_callback_f; 
    plugin->default_parameters_f = default_parameters_f; 
    plugin->initialize_state_f = initialize_state_f;
}

#ifdef DEBUG
extern "C" __declspec(dllexport) 
#endif
void release_jit(Plugin *plugin)
{
    if(plugin->failure_stage == Compiler_Failure_Stage_No_Failure)
    {
        auto *plugin_context = (Plugin_Clang_Context*)plugin->clang_ctx;
        delete plugin_context->jit_engine;
        delete plugin_context->compiler_instance;
        plugin_context->ast_context->Release();
        delete plugin_context;
    }
    plugin->clang_ctx = nullptr;
    plugin->audio_callback_f = nullptr;
    plugin->default_parameters_f = nullptr;
    plugin->initialize_state_f = nullptr;
}

#ifdef DEBUG
extern "C" __declspec(dllexport)
#endif
void release_llvm_ctx(void* llvm_ctx_void)
{
    LLVM_Context *llvm_ctx = (LLVM_Context*)llvm_ctx_void;
    delete llvm_ctx;
}






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
#pragma comment(lib, "clangFrontend.lib")
#pragma comment(lib, "clangFrontendTool.lib")
#pragma comment(lib, "clangHandleCXX.lib")
#pragma comment(lib, "clangHandleLLVM.lib")
#pragma comment(lib, "clangIndex.lib")
#pragma comment(lib, "clangIndexSerialization.lib")
#pragma comment(lib, "clangLex.lib")
#pragma comment(lib, "clangParse.lib")
#pragma comment(lib, "clangRewrite.lib")
#pragma comment(lib, "clangRewriteFrontend.lib")
#pragma comment(lib, "clangSema.lib")
#pragma comment(lib, "clangSerialization.lib")
#pragma comment(lib, "clangStaticAnalyzerCheckers.lib")
#pragma comment(lib, "clangStaticAnalyzerCore.lib")
#pragma comment(lib, "clangStaticAnalyzerFrontend.lib")
#pragma comment(lib, "clangTooling.lib")
#pragma comment(lib, "clangToolingASTDiff.lib")
#pragma comment(lib, "clangToolingCore.lib")
#pragma comment(lib, "clangToolingInclusions.lib")
#pragma comment(lib, "clangToolingRefactoring.lib")
#pragma comment(lib, "clangToolingSyntax.lib")
#pragma comment(lib, "clangToolingASTDiff.lib")

#pragma comment(lib, "clangTransformer.lib")
#pragma comment(lib, "libclang.lib")
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


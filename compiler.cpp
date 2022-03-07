

#include "compiler.h"



extern "C" __declspec(dllexport) void* create_clang_context()
{
    return (void*) create_clang_context_impl();
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

Clang_Context* create_clang_context_impl()
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
    
    auto* clang_ctx = new Clang_Context();
    
    
    llvm::PassBuilder passBuilder;
    
    passBuilder.registerModuleAnalyses(clang_ctx->moduleAnalysisManager);
    passBuilder.registerCGSCCAnalyses(clang_ctx->cGSCCAnalysisManager);
    passBuilder.registerFunctionAnalyses(clang_ctx->functionAnalysisManager);
    clang_ctx->functionAnalysisManager.registerPass([&]{ return passBuilder.buildDefaultAAPipeline(); });
    passBuilder.registerLoopAnalyses(clang_ctx->loopAnalysisManager);
    passBuilder.crossRegisterProxies(clang_ctx->loopAnalysisManager, 
                                     clang_ctx->functionAnalysisManager, 
                                     clang_ctx->cGSCCAnalysisManager, 
                                     clang_ctx->moduleAnalysisManager);
    
    clang_ctx->module_pass_manager = passBuilder.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);
    
    
    
    
    return clang_ctx;
}


extern "C" __declspec(dllexport) Plugin_Handle try_compile(const char* filename, void* clang_ctx_ptr, Compiler_Error_Log *error_log)
{
    return try_compile_impl(filename, (Clang_Context*) clang_ctx_ptr, error_log);
}

void errors_push(Compiler_Error_Log *error_log, Compiler_Error new_error)
{
    if(error_log->count >= error_log->capacity) return; 
    if(new_error.type == Compiler_Error_Type_Success) return;
    if(new_error.type == Compiler_Error_Type_Custom && new_error.custom.flag == Compiler_Success) return; 
    
    error_log->errors[error_log->count++] = new_error;
}


void errors_push_custom(Compiler_Error_Log *error_log, Custom_Error new_error)
{
    errors_push(error_log, {.type = Compiler_Error_Type_Custom, .custom = new_error});  
}


void errors_push_clang(Compiler_Error_Log *error_log, Clang_Error new_error)
{
    errors_push(error_log, {.type = Compiler_Error_Type_Clang, .clang = new_error});  
}


Plugin_Handle try_compile_impl(const char* filename, Clang_Context* clang_ctx, Compiler_Error_Log *error_log)
{
    
    clang::LangOptions language_options{};
    language_options.Bool = 1;
    language_options.CPlusPlus = 1;
    language_options.RTTI = 0;
    language_options.CXXExceptions = 0;
    language_options.MSVCCompat = 1;
    language_options.MicrosoftExt = 1;
    
    clang::CompilerInstance compiler_instance{};
    
    clang::TextDiagnosticBuffer diagnostics{};
    compiler_instance.createDiagnostics(&diagnostics, false);
    
    auto triple = llvm::sys::getDefaultTargetTriple();
    auto triple_str = "-triple=" + triple;
    
    clang::CompilerInvocation::CreateFromArgs(compiler_instance.getInvocation(),
                                              {
                                                  triple_str.c_str(),
                                                  "-Ofast", 
                                                  "-fcxx-exceptions", 
                                                  "-fms-extensions", 
                                                  "-ffast-math", 
                                                  "-fdenormal-fp-math=positive-zero"
                                              }, 
                                              compiler_instance.getDiagnostics());
    
    
    compiler_instance.getTargetOpts().Triple = triple;
    compiler_instance.getLangOpts() = language_options;
    
    auto& header_opts = compiler_instance.getHeaderSearchOpts();
    //header_opts.Verbose = true;
    header_opts.AddPath(".", clang::frontend::Quoted , false, false);
    
    compiler_instance.getFrontendOpts().Inputs.clear();
    auto kind = clang::InputKind(clang::Language::CXX);
    compiler_instance.getFrontendOpts().Inputs.push_back(clang::FrontendInputFile(filename, kind));
    
    bool initial_compilation_succeeded = false;
    
    String plugin_filename =  allocate_and_copy_llvm_stringref(compiler_instance.getFrontendOpts().Inputs.back().getFile().rsplit('\\').second);
    
    Compiler_Error error = { };
    Plugin_Descriptor descriptor = {.name = plugin_filename};
    std::unique_ptr<llvm::MemoryBuffer> new_buffer = nullptr;
    
    auto visit_ast = [&](clang::ASTContext& ast_ctx){
        Plugin_Required_Decls decls = find_decls(ast_ctx);
        if(decls.error.flag != Compiler_Success)
        {
            errors_push_custom(error_log, decls.audio_callback.error);
            errors_push_custom(error_log, decls.default_parameters.error);
            errors_push_custom(error_log, decls.initialize_state.error);
            errors_push_custom(error_log, decls.audio_callback.error);
            errors_push_custom(error_log, decls.audio_callback.error);
            error = {.type = Compiler_Error_Type_Custom, .custom = {Compiler_Could_Not_Get_Decls}};
            return;
        }
        
        descriptor = parse_plugin_descriptor(decls.parameters_struct.record, decls.state_struct.record, ast_ctx.getSourceManager());
        descriptor.name = plugin_filename;
        
        if(descriptor.error.flag != Compiler_Success){
            for(u32 i = 0; i < descriptor.num_parameters; i++)
            {
                errors_push_custom(error_log, descriptor.parameters[i].error);
            }
            error = {.type = Compiler_Error_Type_Custom, .custom = {Compiler_Struct_Parsing_Error}};
            return;
        }
        
        new_buffer = rewrite_plugin_source(decls, 
                                           compiler_instance.getSourceManager(), 
                                           compiler_instance.getLangOpts(), 
                                           compiler_instance.getSourceManager().getMainFileID());
        assert(new_buffer);
        error.type = Compiler_Error_Type_Success;
    };
    
    auto action = make_action(visit_ast);
    compiler_instance.ExecuteAction(action);
    
    
    if(diagnostics.getNumErrors() != 0) 
    {
        for(auto error_it = diagnostics.err_begin(); error_it < diagnostics.err_end(); error_it++)
        {
            errors_push_clang(error_log, {allocate_and_copy_std_string(error_it->second)});
        }
        return Plugin_Handle{
            .error = {Compiler_Clang_Error},
            .descriptor = {},
            .llvm_jit_engine = nullptr,
            .audio_callback_f = nullptr,
            .default_parameters_f = nullptr,
            .initialize_state_f = nullptr
        };
    }
    //succesfully compiled, but failed at some parsing stage
    else if(error.type != Compiler_Error_Type_Success) 
    {
        return Plugin_Handle{
            .error = Compiler_Generic_Error,
            .descriptor = descriptor,
            .llvm_jit_engine = nullptr,
            .audio_callback_f = nullptr,
            .default_parameters_f = nullptr,
            .initialize_state_f = nullptr
        };
    }
    else 
    {
        diagnostics.clear();
        Plugin_Handle handle = jit_compile(new_buffer->getMemBufferRef(), 
                                           compiler_instance, 
                                           descriptor,
                                           &clang_ctx->llvm_context,
                                           clang_ctx->module_pass_manager,
                                           clang_ctx->moduleAnalysisManager,
                                           error_log);
        errors_push_custom(error_log, handle.error);
        return handle;
    }
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
            error = {Compiler_Too_Many_Fun, decl_to_loc(*decl, source_manager)}; 
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
        
        //TODO assert que float est 32 bit
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
            {Compiler_Generic_Error},
            audio_callback_decl,
            default_parameters_decl,
            initialize_state_decl,
            Decl_Handle{.error = Compiler_Generic_Error},
            Decl_Handle{.error = Compiler_Generic_Error}
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
    
    else if(!parameters_type.isRecordType()){
        parameters_struct_decl.error = { Compiler_Not_Record_Type, fun_to_return_loc(*default_parameters_decl.fun, source_manager)};
    }
    else {
        parameters_struct_decl.record = parameters_type.getAsCXXRecordDecl();
        if(parameters_struct_decl.record->isPolymorphic())
        {
            parameters_struct_decl.error = { Compiler_Polymorphic, decl_to_loc(*parameters_struct_decl.record, source_manager)};
        }
    }
    
    Decl_Handle state_struct_decl = {Compiler_Success};
    
    if(&initialize_state_return_type != &audio_callback_state_type)
    {
        state_struct_decl.error = { 
            Compiler_Types_Mismatch, 
            decl_to_loc(*state_struct_decl.record, source_manager), fun_to_return_loc(*initialize_state_decl.fun, source_manager)
        };
    }
    else if(!state_type.isRecordType()){
        state_struct_decl.error = {
            Compiler_Not_Record_Type, 
            decl_to_loc(*state_struct_decl.record, source_manager)
        };
    }
    else {
        state_struct_decl.record = state_type.getAsCXXRecordDecl();
        if(state_struct_decl.record->isPolymorphic())
        {
            state_struct_decl.error = {
                Compiler_Polymorphic,
                decl_to_loc(*state_struct_decl.record, source_manager)
            };
        }
    }
    
    Custom_Error error = 
    ( parameters_struct_decl.error.flag == Compiler_Success && state_struct_decl.error.flag == Compiler_Success ) 
        ? Custom_Error { Compiler_Success } 
    :   Custom_Error{ Compiler_Generic_Error };
    
    return {
        error,
        audio_callback_decl,
        default_parameters_decl,
        initialize_state_decl,
        parameters_struct_decl,
        state_struct_decl
    };
}


Plugin_Descriptor parse_plugin_descriptor(const clang::CXXRecordDecl* parameters_struct_decl, 
                                          const clang::CXXRecordDecl* state_struct_decl,
                                          const clang::SourceManager& source_manager)
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
    auto *anotated_parameters = (Plugin_Descriptor_Parameter*)malloc(sizeof(Plugin_Descriptor_Parameter) * num_annotated_parameters);
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
                                                                   &source_manager] {
                Plugin_Descriptor_Parameter parameter = {};
                
                const auto& type = *field->getType();
                auto index = field->getFieldIndex();
                parameter.offset = parameters_struct_layout.getFieldOffset(index) / 8;
                parameter.name = allocate_and_copy_llvm_stringref(field->getName());
                
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
                    //TODO assert que le getIntegerType soit un int normal
                    for(auto _: maybe_enum->getDecl()->enumerators())
                        enum_param.num_entries++;
                    
                    enum_param.entries = new Parameter_Enum_Entry[enum_param.num_entries];
                    
                    auto i = 0;
                    for(const auto *field : maybe_enum->getDecl()->enumerators())
                    {
                        enum_param.entries[i].name = allocate_and_copy_llvm_stringref(field->getName());
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
        .error = there_was_an_error ? Custom_Error{ Compiler_Generic_Error } : Custom_Error{ Compiler_Success },
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
    
    auto audio_callback_source_range = decls.audio_callback.fun->getSourceRange();
    auto default_parameters_source_range = decls.default_parameters.fun->getSourceRange();
    auto initialize_state_source_range = decls.initialize_state.fun->getSourceRange();
    
    auto parameters_struct_name = decls.parameters_struct.record->getNameAsString();
    auto state_struct_name = decls.state_struct.record->getNameAsString();
    
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
        + parameters_struct_name + "* out_parameters = (" + parameters_struct_name + "*)out_parameters_ptr;\n"
        "*out_parameters = default_parameters();\n"
        "}\n";
    
    std::string initialize_state_wrapper_declaration = "\nextern \"C\" void initialize_state_wrapper(void* parameters, void* out_initial_state, unsigned int num_channels, float sample_rate, void *allocator);\n"
        "";
    
    std::string initialize_state_wrapper_definition = "\n"
        "void initialize_state_wrapper(void* parameters_ptr, void* out_initial_state_ptr, unsigned int num_channels, float sample_rate, void *allocator)\n"
        "{\n"
        + parameters_struct_name + "* parameters = (" + parameters_struct_name + "*)parameters_ptr;\n"
        
        + state_struct_name + "* out_initial_state = (" + state_struct_name + "*)out_initial_state_ptr;\n"
        "*out_initial_state = initialize_state(*parameters, num_channels, sample_rate, allocator);\n"
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
    return llvm::MemoryBuffer::getMemBufferCopy(new_text);
}


Plugin_Handle jit_compile(llvm::MemoryBufferRef new_buffer, clang::CompilerInstance& compiler_instance,
                          Plugin_Descriptor& descriptor,
                          llvm::LLVMContext *llvm_context,
                          llvm::ModulePassManager& module_pass_manager,
                          llvm::ModuleAnalysisManager& module_analysis_manager,
                          Compiler_Error_Log *error_log)
{
    
    auto new_file = clang::FrontendInputFile{
        new_buffer, 
        clang::InputKind{clang::Language::CXX}
    };
    
    compiler_instance.getFrontendOpts().Inputs.clear();
    compiler_instance.getFrontendOpts().Inputs.push_back(new_file);
    
    auto compile_action = std::make_unique<clang::EmitLLVMOnlyAction>(llvm_context);
    
    if (!compiler_instance.ExecuteAction(*compile_action)) 
    {
        //NOTE only possible because we set this ourselves
        auto* diagnostics = static_cast<clang::TextDiagnosticBuffer*>(&compiler_instance.getDiagnosticClient()); 
        assert(diagnostics->getNumErrors() != 0);
        
        for(auto error_it = diagnostics->err_begin(); error_it < diagnostics->err_end(); error_it++)
        {
            errors_push_clang(error_log, {allocate_and_copy_std_string(error_it->second)});
        }
        
        return { Compiler_Clang_Error};
    }
    std::unique_ptr<llvm::Module> module = compile_action->takeModule();
    
    assert(module); 
    //return { Compiler_Cant_Take_Module };
    
    //Optimizations
    module_pass_manager.run(*module, module_analysis_manager);
    
    llvm::EngineBuilder builder(std::move(module));
    builder.setMCJITMemoryManager(std::make_unique<llvm::SectionMemoryManager>());
    builder.setOptLevel(llvm::CodeGenOpt::Level::Aggressive);
    
    llvm::ExecutionEngine *engine = builder.create();
    assert(engine); 
    //return { Compiler_Cant_Launch_Jit };
    
    //on en a pas vraiment besoin de faire ça. C'est dans le cas où on chargerait plusieur modules sur le même executionEngine
    engine->finalizeObject();
    
    auto audio_callback_f = (audio_callback_t)engine->getFunctionAddress("audio_callback_wrapper");
    auto default_parameters_f = (default_parameters_t)engine->getFunctionAddress("default_parameters_wrapper");
    auto initialize_state_f = (initialize_state_t)engine->getFunctionAddress("initialize_state_wrapper");
    
    
    assert(audio_callback_f && default_parameters_f && initialize_state_f);
    
    return Plugin_Handle{
        .error =  { Compiler_Success }, 
        .descriptor = descriptor,
        .llvm_jit_engine = (void*)engine, 
        .audio_callback_f = audio_callback_f, 
        .default_parameters_f = default_parameters_f, 
        .initialize_state_f = initialize_state_f
    };
}

extern "C" __declspec(dllexport) 
void release_jit(Plugin_Handle *plugin)
{
    
    Plugin_Descriptor_Parameter *params = plugin->descriptor.parameters;
    
    delete plugin->descriptor.name.str;
    
    if(plugin->descriptor.num_parameters == 0)
    {
        assert(plugin->descriptor.parameters == nullptr);
    }
    else{
        for(i32 i = 0; i < plugin->descriptor.num_parameters; i++)
        {
            if(params[i].type == Enum)
            {
                if(params[i].enum_param.num_entries != 0)
                {
                    for(i32 j = 0; j < params[i].enum_param.num_entries; j++)
                        delete params[i].enum_param.entries[j].name.str;
                    delete params[i].enum_param.entries;
                }
                else
                {
                    assert(params[i].enum_param.entries == nullptr);
                }
            }
            delete params[i].name.str;
        }
        delete plugin->descriptor.parameters;
    }
    
    
    if(plugin->llvm_jit_engine)
        delete (llvm::ExecutionEngine *)plugin->llvm_jit_engine;
    
    *plugin = {};
    assert(plugin->audio_callback_f == nullptr);
}


extern "C" __declspec(dllexport) 
void release_clang_ctx(void* clang_ctx_void)
{
    Clang_Context *clang_ctx = (Clang_Context*)clang_ctx_void;
    delete clang_ctx;
}
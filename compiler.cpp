

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
    
    llvm::ModulePassManager module_pass_manager = passBuilder.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);
    
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
    
    clang::LangOptions language_options;
    language_options.Bool = 1;
    language_options.CPlusPlus = 1;
    language_options.RTTI = 0;
    language_options.CXXExceptions = 0;
    language_options.MSVCCompat = 1;
    language_options.MicrosoftExt = 1;
    
    
    auto* clang_ctx = new Clang_Context();
    clang_ctx->module_pass_manager = std::move(module_pass_manager);
    clang_ctx->module_analysis_manager = std::move(moduleAnalysisManager);
    
    clang_ctx->compiler_instance.createDiagnostics();
    
    auto triple = llvm::sys::getDefaultTargetTriple();
    auto triple_str = "-triple=" + triple;
    
    clang::CompilerInvocation::CreateFromArgs(clang_ctx->compiler_instance.getInvocation(),
                                              {triple_str.c_str(),"-Ofast", "-fcxx-exceptions", "-fms-extensions", "-ffast-math", "-fdenormal-fp-math=positive-zero"}, 
                                              clang_ctx->compiler_instance.getDiagnostics());
    
    
    clang_ctx->compiler_instance.getTargetOpts().Triple = triple;
    clang_ctx->compiler_instance.getLangOpts() = language_options;
    
    auto& header_opts = clang_ctx->compiler_instance.getHeaderSearchOpts();
    //header_opts.Verbose = true;
    header_opts.AddPath(".", clang::frontend::Quoted , false, false);
    
    return clang_ctx;
}


extern "C" __declspec(dllexport) Plugin_Handle try_compile(const char* filename, void* clang_ctx_ptr)
{
    return try_compile_impl(filename, (Clang_Context*) clang_ctx_ptr);
}

Plugin_Handle try_compile_impl(const char* filename, Clang_Context* clang_ctx)
{
    auto& compiler_instance = clang_ctx->compiler_instance;
    
    compiler_instance.getFrontendOpts().Inputs.clear();
    auto kind = clang::InputKind(clang::Language::CXX);
    compiler_instance.getFrontendOpts().Inputs.push_back(clang::FrontendInputFile(filename, kind));
    
    Plugin_Descriptor descriptor;
    std::unique_ptr<llvm::MemoryBuffer> new_buffer = nullptr;
    {
        //~
        
        bool error = false;
        
        auto visit_ast = [&](clang::ASTContext& ast_ctx){
            auto decls = find_decls(ast_ctx);
            if(!decls.worked)
            {
                error = true;
                return;
            }
            
            if(!parse_plugin_descriptor(decls.parameters_struct, decls.state_struct, descriptor))
            {
                error = true;
                return;
            }
            
            auto& source_manager = compiler_instance.getSourceManager();
            
            new_buffer = rewrite_plugin_source(decls, source_manager, compiler_instance.getLangOpts(), source_manager.getMainFileID());
        };
        
        auto action = make_action(visit_ast);
        compiler_instance.ExecuteAction(action);
        
        if(error == true)
        {
            std::cout << "error while parsing file\n";
            return {false};
        }
        else{
            llvm::outs() << "parsing succeeded\n";
        }
        
    }
    
    return jit_compile(new_buffer->getMemBufferRef(), 
                       compiler_instance, 
                       descriptor,
                       clang_ctx->module_pass_manager,
                       clang_ctx->module_analysis_manager
                       );
}






Plugin_Required_Decls find_decls(clang::ASTContext& ast_ctx)
{
    bool error = false;
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
            std::cout << "too many audio_callback function\n";
            error = true;
        }
    };
    match_ast(audio_callback_matcher, audio_callback_lambda, ast_ctx);
    
    if(audio_callback_decl == nullptr)
    {
        std::cout << "couldn't find an audio_callback function\n";
        return {false};
    }
    
    
    if(error)
        return {false};
    
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
            std::cout << "too many default_parameters function\n";
            error = true;
        }
    };
    match_ast(default_parameters_matcher, default_parameters_lambda, ast_ctx);
    
    if(default_parameters_decl == nullptr)
    {
        std::cout << "couldn't find an default_parameters function\n";
        return {false};
    }
    if(error)
        return {false};
    
    
    
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
            std::cout << "too many initialize_state function\n";
            error = true;
        }
    };
    match_ast(initialize_state_matcher, initialize_state_lambda, ast_ctx);
    
    if(initialize_state_decl == nullptr)
    {
        std::cout << "couldn't find an initialize_state function\n";
        return {false};
    }
    if(error)
        return {false};
    
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
        return {false};
    }
    if(error) 
        return {false};
    
    
    //~
    // check initializer signature
    
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
            std::cout << "fourth parameter should be allocator (unsigned int num_samples) -> void\n"; 
            return false;
        }
        return true;
    }();
    
    if(!initializer_has_the_right_signature)
    {
        return {false};
    }
    
    if(default_parameters_decl->getNumParams() != 0)
    {
        std::cout << "default_parameters doesn't take any parameters\n";
        return {false};
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
        std::cout << "parameter type don't match\n";
        return {false};
    }
    
    if(&initialize_state_return_type != &audio_callback_state_type)
    {
        std::cout << "state types don't match\n";
        return {false};
    }
    
    const auto& state_type = initialize_state_return_type;
    const auto& parameters_type = default_parameters_return_type;
    
    if(!state_type.isRecordType() || !parameters_type.isRecordType())
    {
        std::cout << "must be records\n"; 
        return {false};
    }
    
    const auto* state_struct_decl = state_type.getAsCXXRecordDecl();
    const auto* parameters_struct_decl = parameters_type.getAsCXXRecordDecl();
    
    //TODO check if false
    
    return {
        true,
        audio_callback_decl,
        default_parameters_decl,
        initialize_state_decl,
        parameters_struct_decl,
        state_struct_decl
    };
}







std::unique_ptr<llvm::MemoryBuffer> 
rewrite_plugin_source(Plugin_Required_Decls decls,
                      clang::SourceManager& source_manager,
                      clang::LangOptions& language_options,
                      clang::FileID plugin_source_file_id)
{
    
    auto audio_callback_source_range = decls.audio_callback->getSourceRange();
    auto default_parameters_source_range = decls.default_parameters->getSourceRange();
    auto initialize_state_source_range = decls.initialize_state->getSourceRange();
    
    auto parameters_struct_name = decls.parameters_struct->getNameAsString();
    auto state_struct_name = decls.state_struct->getNameAsString();
    
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
    
    std::string initialize_state_wrapper_declaration = "\nextern \"C\" void initialize_state_wrapper(void* parameters, void* out_initial_state, unsigned int num_channels, float sample_rate, allocator_t allocator);\n"
        "";
    
    std::string initialize_state_wrapper_definition = "\n"
        "void initialize_state_wrapper(void* parameters_ptr, void* out_initial_state_ptr, unsigned int num_channels, float sample_rate, allocator_t allocator)\n"
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














bool parse_plugin_descriptor(const clang::CXXRecordDecl* parameters_struct_decl, 
                             const clang::CXXRecordDecl* state_struct_decl,
                             Plugin_Descriptor& plugin_descriptor)
{
    if(parameters_struct_decl->isPolymorphic() || 
       state_struct_decl->isPolymorphic())
    {
        std::cout << "Cannot be polymorphic\n";
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
                    
                    std::cout << "Error : not an Int type\n";
                    return false;
                }
                
                if(param_strings.size() != 3) 
                {
                    std::cout << "two parameters required : min and max\n";
                    return false;
                }
                
                char* end_ptr = nullptr;
                i32 min = strtol(param_strings[1].c_str() , &end_ptr, 0);
                if(end_ptr == nullptr)
                {
                    std::cout << "invalid min value\n";
                    return false;
                }
                i32 max = strtol(param_strings[2].c_str(), &end_ptr, 0);
                if(end_ptr == nullptr)
                {
                    std::cout << "invalid max value\n";
                    return false;
                }
                if(min >= max)
                {
                    std::cout << "min must be greater than max\n";
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
                    
                    std::cout << "Error : not a Float type\n";
                    return false;
                }
                
                if(param_strings.size() != 3) 
                {
                    std::cout << "two parameters required : min and max\n";
                    return false;
                }
                
                char* end_ptr = nullptr;
                real32 min = strtof(param_strings[1].c_str(), &end_ptr);
                if(end_ptr == nullptr)
                {
                    std::cout << "invalid min value\n";
                    return false;
                }
                real32 max = strtof(param_strings[2].c_str(), &end_ptr);
                if(end_ptr == nullptr)
                {
                    std::cout << "invalid max value\n";
                    return false;
                }
                if(min >= max)
                {
                    std::cout << "min must be greater than max\n";
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
                    
                    std::cout << "Error : not an Enum type\n";
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
                
                std::cout << "Invalid type annotation, should be Int, Float or Enum\n";
                return false;
            }
            
            std::cout << "\n";
            print_parameter(plugin_parameter);
            std::cout << "\n";
            
            valid_parameters[valid_parameter_idx++] = plugin_parameter;
        }
    }
    
    valid_parameters = (Plugin_Descriptor_Parameter*) realloc(valid_parameters, sizeof(Plugin_Descriptor_Parameter) * valid_parameter_idx);
    
    plugin_descriptor.num_parameters = valid_parameter_idx;
    plugin_descriptor.parameters = valid_parameters;
    return true;
}


Plugin_Handle jit_compile(llvm::MemoryBufferRef new_buffer, clang::CompilerInstance& compiler_instance,
                          Plugin_Descriptor& descriptor,
                          llvm::ModulePassManager& module_pass_manager,
                          llvm::ModuleAnalysisManager& module_analysis_manager)
{
    
    auto new_file = clang::FrontendInputFile{
        new_buffer, 
        clang::InputKind{clang::Language::CXX}
    };
    
    compiler_instance.getFrontendOpts().Inputs.clear();
    compiler_instance.getFrontendOpts().Inputs.push_back(new_file);
    
    llvm::LLVMContext llvm_context;
    
    auto compile_action = std::make_unique<clang::EmitLLVMOnlyAction>(&llvm_context);
    
    if (compiler_instance.ExecuteAction(*compile_action)) 
    {
        if(compiler_instance.getDiagnostics().getNumErrors() == 0)
        {
            std::unique_ptr<llvm::Module> module = compile_action->takeModule();
            
            if(module)
            {
                
                //Optimizations
                //module_pass_manager.run(*module, module_analysis_manager);
                
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


#include "compiler_tester/compiler_interface.hpp"
#include <array>
#include <cstdio>
#include <memory>
#include <sstream>
#include <fstream>

namespace compiler_tester {

namespace {

struct CommandResult {
    std::string stdout_output;
    std::string stderr_output;
    int exit_code;
};

auto execute_command_with_output(const std::string& command) -> CommandResult {
    std::array<char, 128> buffer{};
    std::string stdout_result;
    std::string stderr_result;

    std::string full_command = command + " 2>&1";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(full_command.c_str(), "r"), pclose);

    if (!pipe) {
        return {"", "popen() failed", -1};
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        stdout_result += buffer.data();
    }

    int exit_code = pclose(pipe.release()) / 256;

    return {stdout_result, stderr_result, exit_code};
}

auto get_file_size(const std::filesystem::path& path) -> std::size_t {
    if (!std::filesystem::exists(path)) {
        return 0;
    }
    return std::filesystem::file_size(path);
}

} // anonymous namespace

auto CompilerConfig::get_optimization_flag() const -> std::string {
    switch (optimization_level) {
        case OptimizationLevel::O0: return "-O0";
        case OptimizationLevel::O1: return "-O1";
        case OptimizationLevel::O2: return "-O2";
        case OptimizationLevel::O3: return "-O3";
        case OptimizationLevel::Os: return "-Os";
        case OptimizationLevel::Ofast: return "-Ofast";
    }
    return "-O2";
}

// GCCCompiler implementation
GCCCompiler::GCCCompiler(std::filesystem::path binary_path)
    : binary_path_(std::move(binary_path)) {}

auto GCCCompiler::compile(
    const std::filesystem::path& source_file,
    const CompilerConfig& config
) -> CompilationResult {

    std::filesystem::create_directories(config.output_dir);

    auto output_file = config.output_dir /
        (source_file.stem().string() + "_gcc.out");

    std::ostringstream command;
    command << binary_path_.string() << " "
            << config.get_optimization_flag() << " "
            << source_file.string() << " "
            << "-o " << output_file.string();

    if (config.enable_warnings) {
        command << " -Wall -Wextra";
    }

    for (const auto& flag : config.extra_flags) {
        command << " " << flag;
    }

    auto start = std::chrono::steady_clock::now();
    auto result = execute_command_with_output(command.str());
    auto end = std::chrono::steady_clock::now();

    auto compilation_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    return CompilationResult{
        .success = (result.exit_code == 0),
        .binary_path = output_file,
        .stdout_output = std::move(result.stdout_output),
        .stderr_output = std::move(result.stderr_output),
        .binary_size = get_file_size(output_file),
        .compilation_time = compilation_time,
        .exit_code = result.exit_code
    };
}

auto GCCCompiler::get_assembly(
    const std::filesystem::path& source_file,
    const CompilerConfig& config
) -> std::optional<AssemblyCode> {

    auto asm_file = config.output_dir /
        (source_file.stem().string() + "_gcc.s");

    std::ostringstream command;
    command << binary_path_.string() << " "
            << config.get_optimization_flag() << " "
            << "-S " << source_file.string() << " "
            << "-o " << asm_file.string();

    auto result = execute_command_with_output(command.str());

    if (result.exit_code != 0) {
        return std::nullopt;
    }

    std::ifstream file(asm_file);
    if (!file) {
        return std::nullopt;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    auto content = buffer.str();

    return AssemblyCode{
        .content = std::move(content),
        .file_path = asm_file,
        .instruction_count = 0 // Could be computed by parsing
    };
}

auto GCCCompiler::get_version() const -> std::string {
    auto result = execute_command_with_output(binary_path_.string() + " --version");
    return result.stdout_output;
}

auto GCCCompiler::get_type() const noexcept -> CompilerType {
    return CompilerType::GCC;
}

auto GCCCompiler::is_available() const noexcept -> bool {
    return std::filesystem::exists(binary_path_);
}

auto GCCCompiler::supported_optimization_levels() const
    -> std::vector<OptimizationLevel> {
    return {
        OptimizationLevel::O0,
        OptimizationLevel::O1,
        OptimizationLevel::O2,
        OptimizationLevel::O3,
        OptimizationLevel::Os,
        OptimizationLevel::Ofast
    };
}

// ClangCompiler implementation
ClangCompiler::ClangCompiler(std::filesystem::path binary_path)
    : binary_path_(std::move(binary_path)) {}

auto ClangCompiler::compile(
    const std::filesystem::path& source_file,
    const CompilerConfig& config
) -> CompilationResult {

    std::filesystem::create_directories(config.output_dir);

    auto output_file = config.output_dir /
        (source_file.stem().string() + "_clang.out");

    std::ostringstream command;
    command << binary_path_.string() << " "
            << config.get_optimization_flag() << " "
            << source_file.string() << " "
            << "-o " << output_file.string();

    if (config.enable_warnings) {
        command << " -Wall -Wextra";
    }

    for (const auto& flag : config.extra_flags) {
        command << " " << flag;
    }

    auto start = std::chrono::steady_clock::now();
    auto result = execute_command_with_output(command.str());
    auto end = std::chrono::steady_clock::now();

    auto compilation_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    return CompilationResult{
        .success = (result.exit_code == 0),
        .binary_path = output_file,
        .stdout_output = std::move(result.stdout_output),
        .stderr_output = std::move(result.stderr_output),
        .binary_size = get_file_size(output_file),
        .compilation_time = compilation_time,
        .exit_code = result.exit_code
    };
}

auto ClangCompiler::get_assembly(
    const std::filesystem::path& source_file,
    const CompilerConfig& config
) -> std::optional<AssemblyCode> {

    auto asm_file = config.output_dir /
        (source_file.stem().string() + "_clang.s");

    std::ostringstream command;
    command << binary_path_.string() << " "
            << config.get_optimization_flag() << " "
            << "-S " << source_file.string() << " "
            << "-o " << asm_file.string();

    auto result = execute_command_with_output(command.str());

    if (result.exit_code != 0) {
        return std::nullopt;
    }

    std::ifstream file(asm_file);
    if (!file) {
        return std::nullopt;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    auto content = buffer.str();

    return AssemblyCode{
        .content = std::move(content),
        .file_path = asm_file,
        .instruction_count = 0
    };
}

auto ClangCompiler::get_llvm_ir(
    const std::filesystem::path& source_file,
    const CompilerConfig& config
) -> std::optional<std::string> {

    auto ir_file = config.output_dir /
        (source_file.stem().string() + "_clang.ll");

    std::ostringstream command;
    command << binary_path_.string() << " "
            << config.get_optimization_flag() << " "
            << "-S -emit-llvm " << source_file.string() << " "
            << "-o " << ir_file.string();

    auto result = execute_command_with_output(command.str());

    if (result.exit_code != 0) {
        return std::nullopt;
    }

    std::ifstream file(ir_file);
    if (!file) {
        return std::nullopt;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

auto ClangCompiler::get_version() const -> std::string {
    auto result = execute_command_with_output(binary_path_.string() + " --version");
    return result.stdout_output;
}

auto ClangCompiler::get_type() const noexcept -> CompilerType {
    return CompilerType::Clang;
}

auto ClangCompiler::is_available() const noexcept -> bool {
    return std::filesystem::exists(binary_path_);
}

auto ClangCompiler::supported_optimization_levels() const
    -> std::vector<OptimizationLevel> {
    return {
        OptimizationLevel::O0,
        OptimizationLevel::O1,
        OptimizationLevel::O2,
        OptimizationLevel::O3,
        OptimizationLevel::Os,
        OptimizationLevel::Ofast
    };
}

// CompilerFactory implementation
auto CompilerFactory::create_gcc(std::optional<std::filesystem::path> binary_path)
    -> std::unique_ptr<CompilerInterface> {
    auto path = binary_path.value_or("/usr/bin/gcc");
    return std::make_unique<GCCCompiler>(std::move(path));
}

auto CompilerFactory::create_clang(std::optional<std::filesystem::path> binary_path)
    -> std::unique_ptr<CompilerInterface> {
    auto path = binary_path.value_or("/usr/bin/clang");
    return std::make_unique<ClangCompiler>(std::move(path));
}

auto CompilerFactory::detect_available_compilers()
    -> std::vector<std::unique_ptr<CompilerInterface>> {

    std::vector<std::unique_ptr<CompilerInterface>> compilers;

    auto gcc = create_gcc();
    if (gcc->is_available()) {
        compilers.push_back(std::move(gcc));
    }

    auto clang = create_clang();
    if (clang->is_available()) {
        compilers.push_back(std::move(clang));
    }

    return compilers;
}

} // namespace compiler_tester

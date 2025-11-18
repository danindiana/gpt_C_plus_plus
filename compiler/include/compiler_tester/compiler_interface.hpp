#pragma once

#include <chrono>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace compiler_tester {

/// Optimization levels
enum class OptimizationLevel {
    O0,  // No optimization
    O1,  // Basic optimization
    O2,  // Moderate optimization
    O3,  // Aggressive optimization
    Os,  // Optimize for size
    Ofast // Aggressive optimization, may break standards compliance
};

/// Compiler types
enum class CompilerType {
    GCC,
    Clang,
    MSVC,
    ICC,
    Unknown
};

/// Compilation result
struct CompilationResult {
    bool success{false};
    std::filesystem::path binary_path;
    std::string stdout_output;
    std::string stderr_output;
    std::size_t binary_size{0};
    std::chrono::milliseconds compilation_time{0};
    int exit_code{0};

    [[nodiscard]] auto is_valid() const noexcept -> bool {
        return success && std::filesystem::exists(binary_path);
    }
};

/// Compiler configuration
struct CompilerConfig {
    std::filesystem::path compiler_path;
    OptimizationLevel optimization_level{OptimizationLevel::O3};
    std::vector<std::string> extra_flags;
    std::filesystem::path output_dir{"./build"};
    bool generate_assembly{false};
    bool generate_llvm_ir{false};
    bool enable_warnings{true};
    std::optional<std::chrono::seconds> timeout;

    [[nodiscard]] auto get_optimization_flag() const -> std::string;
};

/// Assembly code representation
struct AssemblyCode {
    std::string content;
    std::filesystem::path file_path;
    std::size_t instruction_count{0};

    [[nodiscard]] auto is_valid() const noexcept -> bool {
        return !content.empty();
    }
};

/// Interface for compiler operations
class CompilerInterface {
public:
    virtual ~CompilerInterface() = default;

    /// Compile a source file
    [[nodiscard]] virtual auto compile(
        const std::filesystem::path& source_file,
        const CompilerConfig& config
    ) -> CompilationResult = 0;

    /// Get assembly output
    [[nodiscard]] virtual auto get_assembly(
        const std::filesystem::path& source_file,
        const CompilerConfig& config
    ) -> std::optional<AssemblyCode> = 0;

    /// Get compiler version
    [[nodiscard]] virtual auto get_version() const -> std::string = 0;

    /// Get compiler type
    [[nodiscard]] virtual auto get_type() const noexcept -> CompilerType = 0;

    /// Check if compiler is available
    [[nodiscard]] virtual auto is_available() const noexcept -> bool = 0;

    /// Get supported optimization levels
    [[nodiscard]] virtual auto supported_optimization_levels() const
        -> std::vector<OptimizationLevel> = 0;
};

/// GCC compiler implementation
class GCCCompiler final : public CompilerInterface {
public:
    explicit GCCCompiler(std::filesystem::path binary_path = "/usr/bin/gcc");

    [[nodiscard]] auto compile(
        const std::filesystem::path& source_file,
        const CompilerConfig& config
    ) -> CompilationResult override;

    [[nodiscard]] auto get_assembly(
        const std::filesystem::path& source_file,
        const CompilerConfig& config
    ) -> std::optional<AssemblyCode> override;

    [[nodiscard]] auto get_version() const -> std::string override;
    [[nodiscard]] auto get_type() const noexcept -> CompilerType override;
    [[nodiscard]] auto is_available() const noexcept -> bool override;

    [[nodiscard]] auto supported_optimization_levels() const
        -> std::vector<OptimizationLevel> override;

private:
    std::filesystem::path binary_path_;
};

/// Clang compiler implementation
class ClangCompiler final : public CompilerInterface {
public:
    explicit ClangCompiler(std::filesystem::path binary_path = "/usr/bin/clang");

    [[nodiscard]] auto compile(
        const std::filesystem::path& source_file,
        const CompilerConfig& config
    ) -> CompilationResult override;

    [[nodiscard]] auto get_assembly(
        const std::filesystem::path& source_file,
        const CompilerConfig& config
    ) -> std::optional<AssemblyCode> override;

    [[nodiscard]] auto get_version() const -> std::string override;
    [[nodiscard]] auto get_type() const noexcept -> CompilerType override;
    [[nodiscard]] auto is_available() const noexcept -> bool override;

    [[nodiscard]] auto supported_optimization_levels() const
        -> std::vector<OptimizationLevel> override;

    /// Get LLVM IR output
    [[nodiscard]] auto get_llvm_ir(
        const std::filesystem::path& source_file,
        const CompilerConfig& config
    ) -> std::optional<std::string>;

private:
    std::filesystem::path binary_path_;
};

/// Factory for creating compiler interfaces
class CompilerFactory {
public:
    [[nodiscard]] static auto create_gcc(
        std::optional<std::filesystem::path> binary_path = std::nullopt
    ) -> std::unique_ptr<CompilerInterface>;

    [[nodiscard]] static auto create_clang(
        std::optional<std::filesystem::path> binary_path = std::nullopt
    ) -> std::unique_ptr<CompilerInterface>;

    [[nodiscard]] static auto detect_available_compilers()
        -> std::vector<std::unique_ptr<CompilerInterface>>;
};

} // namespace compiler_tester

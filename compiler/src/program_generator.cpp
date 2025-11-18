#include "compiler_tester/program_generator.hpp"
#include <array>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace compiler_tester {

namespace {

auto execute_command(const std::string& command) -> std::pair<std::string, int> {
    std::array<char, 128> buffer{};
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return {result, pclose(pipe.release())};
}

auto read_file(const std::filesystem::path& path) -> std::string {
    std::ifstream file(path);
    if (!file) {
        return {};
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

auto write_file(const std::filesystem::path& path, std::string_view content) -> bool {
    std::ofstream file(path);
    if (!file) {
        return false;
    }
    file << content;
    return file.good();
}

} // anonymous namespace

// CsmithGenerator implementation
CsmithGenerator::CsmithGenerator(std::filesystem::path csmith_binary)
    : binary_path_(std::move(csmith_binary)) {}

auto CsmithGenerator::generate(const GeneratorConfig& config)
    -> std::optional<GeneratedProgram> {

    if (!is_available()) {
        return std::nullopt;
    }

    std::filesystem::create_directories(config.output_dir);

    std::ostringstream command;
    command << binary_path_.string();

    if (config.deterministic) {
        command << " --seed " << config.seed;
    }

    command << " --max-funcs " << config.max_functions;

    auto [output, exit_code] = execute_command(command.str());

    if (exit_code != 0) {
        return std::nullopt;
    }

    auto output_file = config.output_dir /
        ("csmith_" + std::to_string(config.seed) + ".c");

    if (!write_file(output_file, output)) {
        return std::nullopt;
    }

    return GeneratedProgram{
        .file_path = output_file,
        .source_code = std::move(output),
        .seed = config.seed,
        .generator_name = "Csmith"
    };
}

auto CsmithGenerator::generate_batch(const GeneratorConfig& config, std::size_t count)
    -> std::vector<GeneratedProgram> {

    std::vector<GeneratedProgram> programs;
    programs.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        auto modified_config = config;
        modified_config.seed = config.seed + i;

        if (auto program = generate(modified_config)) {
            programs.push_back(std::move(*program));
        }
    }

    return programs;
}

auto CsmithGenerator::is_available() const noexcept -> bool {
    return std::filesystem::exists(binary_path_) &&
           std::filesystem::is_regular_file(binary_path_);
}

// YarpGenGenerator implementation
YarpGenGenerator::YarpGenGenerator(std::filesystem::path yarpgen_binary)
    : binary_path_(std::move(yarpgen_binary)) {}

auto YarpGenGenerator::generate(const GeneratorConfig& config)
    -> std::optional<GeneratedProgram> {

    if (!is_available()) {
        return std::nullopt;
    }

    std::filesystem::create_directories(config.output_dir);

    auto work_dir = config.output_dir / ("yarpgen_" + std::to_string(config.seed));
    std::filesystem::create_directories(work_dir);

    std::ostringstream command;
    command << "cd " << work_dir.string() << " && "
            << binary_path_.string();

    if (config.deterministic) {
        command << " --seed=" << config.seed;
    }

    auto [output, exit_code] = execute_command(command.str());

    if (exit_code != 0) {
        return std::nullopt;
    }

    // YarpGen typically creates func.cpp
    auto output_file = work_dir / "func.cpp";

    if (!std::filesystem::exists(output_file)) {
        return std::nullopt;
    }

    auto source_code = read_file(output_file);

    return GeneratedProgram{
        .file_path = output_file,
        .source_code = std::move(source_code),
        .seed = config.seed,
        .generator_name = "YarpGen"
    };
}

auto YarpGenGenerator::generate_batch(const GeneratorConfig& config, std::size_t count)
    -> std::vector<GeneratedProgram> {

    std::vector<GeneratedProgram> programs;
    programs.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        auto modified_config = config;
        modified_config.seed = config.seed + i;

        if (auto program = generate(modified_config)) {
            programs.push_back(std::move(*program));
        }
    }

    return programs;
}

auto YarpGenGenerator::is_available() const noexcept -> bool {
    return std::filesystem::exists(binary_path_) &&
           std::filesystem::is_regular_file(binary_path_);
}

// GeneratorFactory implementation
auto GeneratorFactory::create_csmith(std::optional<std::filesystem::path> binary_path)
    -> std::unique_ptr<ProgramGenerator> {

    auto path = binary_path.value_or("/usr/bin/csmith");
    return std::make_unique<CsmithGenerator>(std::move(path));
}

auto GeneratorFactory::create_yarpgen(std::optional<std::filesystem::path> binary_path)
    -> std::unique_ptr<ProgramGenerator> {

    auto path = binary_path.value_or("/usr/bin/yarpgen");
    return std::make_unique<YarpGenGenerator>(std::move(path));
}

auto GeneratorFactory::create_default() -> std::unique_ptr<ProgramGenerator> {
    // Try Csmith first, fall back to YarpGen
    auto csmith = create_csmith();
    if (csmith->is_available()) {
        return csmith;
    }

    auto yarpgen = create_yarpgen();
    if (yarpgen->is_available()) {
        return yarpgen;
    }

    throw std::runtime_error("No program generator available");
}

} // namespace compiler_tester

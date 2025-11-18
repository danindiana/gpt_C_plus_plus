#include "compiler_tester/program_generator.hpp"
#include "compiler_tester/information_injector.hpp"
#include "compiler_tester/compiler_interface.hpp"
#include "compiler_tester/oracle.hpp"
#include "compiler_tester/reporter.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace ct = compiler_tester;

void print_usage(const char* program_name) {
    std::cout << "Compiler Optimization Tester v1.0.0\n\n";
    std::cout << "Usage: " << program_name << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --help, -h              Show this help message\n";
    std::cout << "  --version, -v           Show version information\n";
    std::cout << "  --generate <count>      Generate test programs (default: 10)\n";
    std::cout << "  --compiler <name>       Specify compiler (gcc, clang, both)\n";
    std::cout << "  --output-dir <path>     Output directory (default: ./output)\n";
    std::cout << "  --optimization <level>  Optimization level (O0, O1, O2, O3, Os, Ofast)\n";
    std::cout << "\nExample:\n";
    std::cout << "  " << program_name << " --generate 5 --compiler both --optimization O3\n";
}

void print_version() {
    std::cout << "Compiler Optimization Tester v1.0.0\n";
    std::cout << "Built with C++23\n";
    std::cout << "Based on research: 'Refined Input, Degraded Output' (PLDI 2024)\n";
}

auto parse_optimization_level(std::string_view level) -> ct::OptimizationLevel {
    if (level == "O0") return ct::OptimizationLevel::O0;
    if (level == "O1") return ct::OptimizationLevel::O1;
    if (level == "O2") return ct::OptimizationLevel::O2;
    if (level == "O3") return ct::OptimizationLevel::O3;
    if (level == "Os") return ct::OptimizationLevel::Os;
    if (level == "Ofast") return ct::OptimizationLevel::Ofast;
    return ct::OptimizationLevel::O3; // default
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    std::size_t program_count = 10;
    std::string compiler_choice = "both";
    std::string output_dir = "./output";
    ct::OptimizationLevel opt_level = ct::OptimizationLevel::O3;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        }
        else if (arg == "--version" || arg == "-v") {
            print_version();
            return 0;
        }
        else if (arg == "--generate" && i + 1 < argc) {
            program_count = std::stoul(argv[++i]);
        }
        else if (arg == "--compiler" && i + 1 < argc) {
            compiler_choice = argv[++i];
        }
        else if (arg == "--output-dir" && i + 1 < argc) {
            output_dir = argv[++i];
        }
        else if (arg == "--optimization" && i + 1 < argc) {
            opt_level = parse_optimization_level(argv[++i]);
        }
    }

    std::cout << "=== Compiler Optimization Inconsistency Tester ===\n\n";

    // Setup components
    ct::GeneratorConfig gen_config;
    gen_config.output_dir = output_dir + "/generated";
    gen_config.seed = 42;

    ct::InjectorConfig inj_config;
    ct::OracleConfig oracle_config;
    ct::ReporterConfig reporter_config;
    reporter_config.output_dir = output_dir + "/reports";

    ct::CompilerConfig compiler_config;
    compiler_config.optimization_level = opt_level;
    compiler_config.output_dir = output_dir + "/build";

    // Create instances
    ct::InformationInjector injector(inj_config);
    ct::Reporter reporter(reporter_config);
    auto oracle = ct::OracleFactory::create_default_oracle();

    // Detect available compilers
    auto compilers = ct::CompilerFactory::detect_available_compilers();

    if (compilers.empty()) {
        std::cerr << "Error: No compilers found!\n";
        std::cerr << "Please ensure GCC or Clang is installed.\n";
        return 1;
    }

    std::cout << "Found " << compilers.size() << " compiler(s):\n";
    for (const auto& compiler : compilers) {
        std::cout << "  - " << compiler->get_version().substr(0, 50) << "...\n";
    }
    std::cout << "\n";

    // Try to create a program generator
    std::unique_ptr<ct::ProgramGenerator> generator;

    try {
        generator = ct::GeneratorFactory::create_default();
        std::cout << "Using program generator: " << generator->name() << "\n\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Warning: " << e.what() << "\n";
        std::cerr << "No program generator available. Using example mode.\n\n";
    }

    // Statistics
    ct::TestingStatistics stats;
    stats.start_time = std::chrono::system_clock::now();

    if (generator) {
        std::cout << "Generating " << program_count << " test program(s)...\n";

        for (std::size_t i = 0; i < program_count; ++i) {
            std::cout << "  [" << (i + 1) << "/" << program_count << "] ";

            auto modified_config = gen_config;
            modified_config.seed = gen_config.seed + i;

            auto program = generator->generate(modified_config);

            if (!program) {
                std::cout << "Failed to generate program\n";
                continue;
            }

            std::cout << "Generated: " << program->file_path.filename().string() << "\n";

            // Create a simple test by injecting dummy information
            ct::ProgramInformation info;
            // In real usage, would derive information from program execution

            auto injection_result = injector.inject_all(program->file_path, info);

            if (!injection_result.is_valid()) {
                std::cout << "    Failed to inject information\n";
                continue;
            }

            // Test with each compiler
            for (const auto& compiler : compilers) {
                auto original = compiler->compile(program->file_path, compiler_config);
                auto refined = compiler->compile(injection_result.refined_file, compiler_config);

                stats.total_programs_tested++;

                if (auto inconsistency = oracle->check(original, refined, oracle_config)) {
                    std::cout << "    Found inconsistency with "
                              << (compiler->get_type() == ct::CompilerType::GCC ? "GCC" : "Clang")
                              << "!\n";

                    inconsistency->compiler = compiler->get_type();
                    inconsistency->compiler_version = compiler->get_version();

                    reporter.add_inconsistency(*inconsistency);
                    stats.total_inconsistencies_found++;
                }
            }
        }
    } else {
        std::cout << "Running in demo mode (no test generation).\n";
        std::cout << "To enable test generation, install Csmith or YarpGen.\n\n";
    }

    stats.end_time = std::chrono::system_clock::now();

    // Generate report
    std::cout << "\n=== Testing Complete ===\n\n";
    std::cout << "Programs tested: " << stats.total_programs_tested << "\n";
    std::cout << "Inconsistencies found: " << stats.total_inconsistencies_found << "\n";
    std::cout << "Duration: " << stats.duration().count() << " seconds\n\n";

    if (stats.total_inconsistencies_found > 0) {
        auto report_path = reporter.generate_report(stats);
        std::cout << "Report saved to: " << report_path << "\n";
    }

    return 0;
}

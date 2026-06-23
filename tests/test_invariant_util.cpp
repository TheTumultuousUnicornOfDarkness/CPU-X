#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>

// Forward declaration of the vulnerable function from src/util.cpp
extern int vulnerable_function(const char* user_input, char** output);

class SecurityTest : public ::testing::TestWithParam<std::string> {};

TEST_P(SecurityTest, ShellCommandInjectionResistant) {
    // Invariant: User input must not allow shell command injection
    std::string payload = GetParam();
    char* output = nullptr;
    
    // Call the actual production function
    int result = vulnerable_function(payload.c_str(), &output);
    
    // Security property: output must not contain unexpected shell metacharacters
    // or the function must safely reject/escape the input
    if (output != nullptr) {
        // Check that no shell metacharacters appear in dangerous positions
        // This is a simplified check - real implementation would be more robust
        std::string output_str(output);
        bool has_injection = output_str.find(";") != std::string::npos ||
                             output_str.find("&") != std::string::npos ||
                             output_str.find("|") != std::string::npos ||
                             output_str.find("`") != std::string::npos ||
                             output_str.find("$(") != std::string::npos;
        
        EXPECT_FALSE(has_injection) << "Shell injection detected in output for input: " << payload;
        free(output);
    }
}

INSTANTIATE_TEST_SUITE_P(
    AdversarialInputs,
    SecurityTest,
    ::testing::Values(
        // Exact exploit case - command injection
        "test; rm -rf /",
        // Boundary case - input with multiple metacharacters
        "valid_input && cat /etc/passwd",
        // Valid input - should pass
        "ls -la",
        // Another adversarial case - subshell injection
        "test$(whoami)",
        // Edge case - newline injection
        "test\nreboot"
    )
);

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
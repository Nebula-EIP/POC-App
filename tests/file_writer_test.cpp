#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "file-writing/file-writer.hpp"

namespace {
namespace fs = std::filesystem;
using file_writing::FileWriter;

// Base directory for all test-generated files.
static const fs::path kTestDir = fs::temp_directory_path() / "nebula_file_writer_tests";

class FileWriterTest : public ::testing::Test {
 protected:
    void SetUp() override
    {
        // Ensure a clean test directory before each test.
        fs::remove_all(kTestDir);
    }

    void TearDown() override
    {
        // Clean up everything created during the test.
        fs::remove_all(kTestDir);
    }

    /// Helper: read back the entire contents of a file.
    static std::string ReadAll(const fs::path& path)
    {
        std::ifstream in(path);
        return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    }
};

TEST_F(FileWriterTest, WritesContentToNewFile)
{
    FileWriter writer;
    fs::path file = kTestDir / "hello.txt";

    EXPECT_TRUE(writer.WriteToFile(file.string(), "Hello, World!"));
    EXPECT_TRUE(fs::exists(file));
    EXPECT_EQ(ReadAll(file), "Hello, World!");
}

TEST_F(FileWriterTest, OverwritesExistingFile)
{
    FileWriter writer;
    fs::path file = kTestDir / "overwrite.txt";

    writer.WriteToFile(file.string(), "first");
    writer.WriteToFile(file.string(), "second");

    EXPECT_EQ(ReadAll(file), "second");
}

TEST_F(FileWriterTest, CreatesParentDirectories)
{
    FileWriter writer;
    fs::path file = kTestDir / "a" / "b" / "c" / "deep.txt";

    EXPECT_TRUE(writer.WriteToFile(file.string(), "deep content"));
    EXPECT_TRUE(fs::exists(file));
    EXPECT_EQ(ReadAll(file), "deep content");
}

TEST_F(FileWriterTest, WritesEmptyContent)
{
    FileWriter writer;
    fs::path file = kTestDir / "empty.txt";

    EXPECT_TRUE(writer.WriteToFile(file.string(), ""));
    EXPECT_TRUE(fs::exists(file));
    EXPECT_TRUE(ReadAll(file).empty());
}

TEST_F(FileWriterTest, WritesMultilineContent)
{
    FileWriter writer;
    fs::path file = kTestDir / "multi.cpp";

    const std::string code =
        "#include <iostream>\n"
        "int main() {\n"
        "    return 0;\n"
        "}\n";

    EXPECT_TRUE(writer.WriteToFile(file.string(), code));
    EXPECT_EQ(ReadAll(file), code);
}

}  // namespace

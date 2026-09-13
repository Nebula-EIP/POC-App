#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "file-writing/file-writer.hpp"

namespace {
namespace fs = std::filesystem;
using file_writing::FileWriter;

class FileWriterTest : public ::testing::Test {
 protected:
    fs::path test_dir_;

    fs::path GetTestDirectory() const
    {
        const auto* test_info = ::testing::UnitTest::GetInstance()->current_test_info();
        fs::path base_dir = fs::temp_directory_path() / "nebula_file_writer_tests";

        if (test_info != nullptr) {
            return base_dir / test_info->test_suite_name() / test_info->name();
        }

        return base_dir / "unknown_test";
    }

    void SetUp() override
    {
        // Use an isolated temp folder per test to avoid cross-test races in parallel runs.
        test_dir_ = GetTestDirectory();
        fs::remove_all(test_dir_);
        fs::create_directories(test_dir_);
    }

    void TearDown() override
    {
        // Clean up everything created during the test.
        fs::remove_all(test_dir_);
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
    fs::path file = test_dir_ / "hello.txt";

    EXPECT_TRUE(writer.WriteToFile(file.string(), "Hello, World!"));
    EXPECT_TRUE(fs::exists(file));
    EXPECT_EQ(ReadAll(file), "Hello, World!");
}

TEST_F(FileWriterTest, OverwritesExistingFile)
{
    FileWriter writer;
    fs::path file = test_dir_ / "overwrite.txt";

    writer.WriteToFile(file.string(), "first");
    writer.WriteToFile(file.string(), "second");

    EXPECT_EQ(ReadAll(file), "second");
}

TEST_F(FileWriterTest, CreatesParentDirectories)
{
    FileWriter writer;
    fs::path file = test_dir_ / "a" / "b" / "c" / "deep.txt";

    EXPECT_TRUE(writer.WriteToFile(file.string(), "deep content"));
    EXPECT_TRUE(fs::exists(file));
    EXPECT_EQ(ReadAll(file), "deep content");
}

TEST_F(FileWriterTest, WritesEmptyContent)
{
    FileWriter writer;
    fs::path file = test_dir_ / "empty.txt";

    EXPECT_TRUE(writer.WriteToFile(file.string(), ""));
    EXPECT_TRUE(fs::exists(file));
    EXPECT_TRUE(ReadAll(file).empty());
}

TEST_F(FileWriterTest, WritesMultilineContent)
{
    FileWriter writer;
    fs::path file = test_dir_ / "multi.cpp";

    const std::string code =
        "#include <iostream>\n"
        "int main() {\n"
        "    return 0;\n"
        "}\n";

    EXPECT_TRUE(writer.WriteToFile(file.string(), code));
    EXPECT_EQ(ReadAll(file), code);
}

}  // namespace

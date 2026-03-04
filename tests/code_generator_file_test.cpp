#include <gtest/gtest.h>

#include <string>

#include "code-generation/code-generator-errors.hpp"
#include "code-generation/code-generator-file.hpp"

namespace {
using code_generation::CodeGeneratorFile;

TEST(CodeGeneratorFileTest, LineAppendsAtCursorAndMovesCursor)
{
    CodeGeneratorFile gen;

    EXPECT_EQ(gen.Line("A"), 1);
    EXPECT_EQ(gen.Line("B"), 2);
    EXPECT_EQ(gen.GetContent(), "AB");
}

TEST(CodeGeneratorFileTest, LineAtInsertsAtBeginningWithoutReorderingExisting)
{
    CodeGeneratorFile gen;

    gen.Line("B");
    gen.Line("C");
    gen.LineAt("A", 0);

    EXPECT_EQ(gen.GetContent(), "ABC");
}

TEST(CodeGeneratorFileTest, LineAtWrapsPositions)
{
    CodeGeneratorFile gen;

    gen.Line("A");
    gen.Line("B");

    // With 2 lines in content, valid insert positions are [0..2].
    // position = -1 wraps to 2 (end)
    gen.LineAt("X", -1);
    EXPECT_EQ(gen.GetContent(), "ABX");

    // content size is now 3, size+1 == 4, position=4 wraps to 0
    gen.LineAt("Y", 4);
    EXPECT_EQ(gen.GetContent(), "YABX");
}

TEST(CodeGeneratorFileTest, CloseBlockFailsIfNoneOpen)
{
    CodeGeneratorFile gen;
    EXPECT_FALSE(gen.CloseBlock());
    EXPECT_EQ(gen.GetContent(), "");
}

TEST(CodeGeneratorFileTest, GetPositionStartBlockReturnsMinusOneWhenNoBlockOpen)
{
    CodeGeneratorFile gen;
    EXPECT_EQ(gen.GetPositionStartBlock(), -1);
}

TEST(CodeGeneratorFileTest, OpenAndCloseBlockGenerateBraces)
{
    CodeGeneratorFile gen;

    EXPECT_TRUE(gen.OpenBlock("int main()"));
    gen.Line("return 0;");
    EXPECT_TRUE(gen.CloseBlock());

    EXPECT_EQ(gen.GetContent(), "int main() {return 0;}");
}

TEST(CodeGeneratorFileTest, GetPositionStartBlockAllowsInsertionInsideBlockWithoutReordering)
{
    CodeGeneratorFile gen;

    gen.OpenBlock("int main()");
    gen.Line("return 0;");

    // Insert before the current cursor (inside the block).
    gen.LineAt("int x = 5;", gen.GetPositionStartBlock());

    // Regression: previously, LineAt() didn't move the cursor, so CloseBlock()
    // inserted '}' before 'return 0;'.
    gen.CloseBlock();

    EXPECT_EQ(gen.GetContent(), "int main() {int x = 5;return 0;}");
}

TEST(CodeGeneratorFileTest, SetIndentLevelRejectsNonPositive)
{
    CodeGeneratorFile gen;

    EXPECT_THROW(gen.SetIndentLevel(0), code_generation::IndentationError);
    EXPECT_THROW(gen.SetIndentLevel(-4), code_generation::IndentationError);
}

TEST(CodeGeneratorFileTest, FormattedContentIndentsAndAddsNewlines)
{
    CodeGeneratorFile gen;

    gen.OpenBlock("int main()");
    gen.Line("return 0;");
    gen.CloseBlock();

    EXPECT_EQ(gen.GetFormatedContent(), "int main() {\n    return 0;\n}\n");
}

TEST(CodeGeneratorFileTest, FormattingIgnoresBracesAndSemicolonsInsideQuotes)
{
    CodeGeneratorFile gen;

    gen.OpenBlock("int main()");
    gen.Line("std::cout << \"H;ello, {World}!\" << std::endl;");
    gen.Line("return 0;");
    gen.CloseBlock();

    // The string literal contains ';' and '{' '}', which must not change indentation
    // nor be treated as split points.
    const std::string formatted = gen.GetFormatedContent();

    EXPECT_NE(formatted.find("    std::cout << \"H;ello, {World}!\" << std::endl;\n"), std::string::npos);
    EXPECT_NE(formatted.find("    return 0;\n"), std::string::npos);

    // Should still have exactly one opening brace line and one closing brace line.
    EXPECT_NE(formatted.find("int main() {\n"), std::string::npos);
    EXPECT_NE(formatted.find("}\n"), std::string::npos);
}

TEST(CodeGeneratorFileTest, SetIndentLevelAffectsFormattedIndent)
{
    CodeGeneratorFile gen;
    gen.SetIndentLevel(2);

    gen.OpenBlock("int main()");
    gen.Line("return 0;");
    gen.CloseBlock();

    EXPECT_EQ(gen.GetFormatedContent(), "int main() {\n  return 0;\n}\n");
}

TEST(CodeGeneratorFileTest, CursorMoveAndSetCursorAffectInsertionPoint)
{
    CodeGeneratorFile gen;

    gen.Line("A");
    gen.Line("C");

    gen.SetCursor(1);
    gen.Line("B");

    EXPECT_EQ(gen.GetContent(), "ABC");

    // For a content size of 3, GetContainedPosition wraps using (size + 1) == 4.
    // So cursor = -4 wraps to position 0 (beginning).
    gen.MoveCursor(-6);
    gen.Line("Z");

    // With negative cursor, Line() routes through LineAt which wraps it into range.
    EXPECT_EQ(gen.GetContent().size(), std::string("ZABC").size());
    EXPECT_EQ(gen.GetContent(), "ZABC");
}

// ---------- CloseAllBlocks ----------

TEST(CodeGeneratorFileTest, CloseAllBlocksReturnsFalseWhenNoBlocksOpen)
{
    CodeGeneratorFile gen;
    EXPECT_FALSE(gen.CloseAllBlocks());
    EXPECT_EQ(gen.GetContent(), "");
}

TEST(CodeGeneratorFileTest, CloseAllBlocksClosesEveryOpenBlock)
{
    CodeGeneratorFile gen;

    gen.OpenBlock("int main()");
    gen.OpenBlock("if (true)");
    gen.Line("x++;");
    EXPECT_TRUE(gen.CloseAllBlocks());

    EXPECT_EQ(gen.GetContent(), "int main() {if (true) {x++;}}");
}

// ---------- CloseAndOpenBlock ----------

TEST(CodeGeneratorFileTest, CloseAndOpenBlockReturnsFalseWhenNoBlocksOpen)
{
    CodeGeneratorFile gen;
    EXPECT_FALSE(gen.CloseAndOpenBlock("else"));
}

TEST(CodeGeneratorFileTest, CloseAndOpenBlockClosesAndReopens)
{
    CodeGeneratorFile gen;

    gen.OpenBlock("if (x > 0)");
    gen.Line("return 1;");
    gen.CloseAndOpenBlock("else");
    gen.Line("return 0;");
    gen.CloseBlock();

    EXPECT_EQ(gen.GetContent(), "if (x > 0) {return 1;}else {return 0;}");
}

// ---------- Nested blocks formatting ----------

TEST(CodeGeneratorFileTest, FormattedNestedBlocks)
{
    CodeGeneratorFile gen;

    gen.OpenBlock("int main()");
    gen.OpenBlock("if (true)");
    gen.Line("x++;");
    gen.CloseAllBlocks();

    const std::string expected =
        "int main() {\n"
        "    if (true) {\n"
        "        x++;\n"
        "    }\n"
        "}\n";

    EXPECT_EQ(gen.GetFormatedContent(), expected);
}

TEST(CodeGeneratorFileTest, FormattedCloseAndOpenBlock)
{
    CodeGeneratorFile gen;

    gen.OpenBlock("if (x > 0)");
    gen.Line("return 1;");
    gen.CloseAndOpenBlock("else");
    gen.Line("return 0;");
    gen.CloseBlock();

    const std::string expected =
        "if (x > 0) {\n"
        "    return 1;\n"
        "}\n"
        "else {\n"
        "    return 0;\n"
        "}\n";

    EXPECT_EQ(gen.GetFormatedContent(), expected);
}

// ---------- GetPositionStartBlock with nesting ----------

TEST(CodeGeneratorFileTest, GetPositionStartBlockReturnsInnerBlockStart)
{
    CodeGeneratorFile gen;

    gen.OpenBlock("int main()");
    gen.Line("int x = 0;");
    gen.OpenBlock("if (x)");

    // Should point right after "if (x) {", i.e. inside the inner block.
    int pos = gen.GetPositionStartBlock();
    gen.LineAt("x++;", pos);
    gen.CloseAllBlocks();

    EXPECT_EQ(gen.GetContent(), "int main() {int x = 0;if (x) {x++;}}");}

}  // namespace

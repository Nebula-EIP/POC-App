#include <gtest/gtest.h>
#include "modules/capabilities/type_list_capability.hpp"

using namespace core;
using namespace core::capa;

TEST(TypeListCapabilityTest, RegisterAndRetrieveTypes) {
    TypeListCapability capability;

    capability.RegisterType("int");
    capability.RegisterType("float");

    auto name_ptr1 = capability.registerType(10);
    ASSERT_NE(name_ptr1, nullptr);
    EXPECT_EQ(*name_ptr1, "int");

    auto name_ptr2 = capability.registerType(20);
    ASSERT_NE(name_ptr2, nullptr);
    EXPECT_EQ(*name_ptr2, "float");

    // All pending types assigned
    auto name_ptr3 = capability.registerType(30);
    EXPECT_EQ(name_ptr3, nullptr);

    EXPECT_EQ(capability.typeId("int"), 10);
    EXPECT_EQ(capability.typeId("float"), 20);
    EXPECT_EQ(capability.typeId("string"), 0);

    EXPECT_EQ(capability.typeName(10), "int");
    EXPECT_EQ(capability.typeName(20), "float");
    EXPECT_EQ(capability.typeName(30), "");

    auto types = capability.types();
    ASSERT_EQ(types.size(), 2);
    EXPECT_EQ(types[0].id, 10);
    EXPECT_EQ(types[0].name, "int");
    EXPECT_EQ(types[1].id, 20);
    EXPECT_EQ(types[1].name, "float");
}

TEST(TypeListCapabilityTest, DuplicateTypeRegistration) {
    TypeListCapability capability;
    
    capability.RegisterType("int");
    capability.RegisterType("int"); // Should be ignored

    auto name_ptr1 = capability.registerType(10);
    ASSERT_NE(name_ptr1, nullptr);
    EXPECT_EQ(*name_ptr1, "int");

    auto name_ptr2 = capability.registerType(20);
    EXPECT_EQ(name_ptr2, nullptr); // Only one type was registered
}

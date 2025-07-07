
#include <gtest/gtest.h>
#include "User.h"
#include <memory>
#include <string>
#include <vector>
#include <json/json.h>
#include <json/json.h> 
using namespace testing;

class UserTest : public ::testing::Test {
protected:
    std::unique_ptr<User> user = std::make_unique<User>();

    void SetUp() override {
        user->setUsername("testUser");
        user->setPassword("testPass");
        user->setId(1);
    }

    void TearDown() override {}
};

TEST_F(UserTest, User_Login_Sucessful) {
    // Act
    auto result = user->login();

    // Assert
    EXPECT_TRUE(result.success);
    ASSERT_NOT_NULL(result.data);
}

TEST_F(UserTest, User_Login_Failure) {
    // Arrange
    user->setUsername("invalidUser");

    // Act
    auto result = user->login();

    // Assert
    EXPECT_FALSE(result.success);
}

TEST_F(UserTest, User_JSON_Serialization) {
    // Arrange
    std::string expectedJson = R"({"username":"testUser","password":"testPass","id":1})";

    // Act
    std::string jsonResult = user->toJson();

    // Assert
    EXPECT_EQ(jsonResult, expectedJson);
}

TEST_F(UserTest, User_Password_Encryption) {
    // Act
    auto encrypted = user->encryptPassword("testPass");

    // Assert
    ASSERT_EQ(encrypted.size(), 60); 
    const std::vector<std::string>& bcryptMethods = Utils::getSupportedBcryptMethods();
    EXPECT_TRUE(std::any_of(bcryptMethods.begin(), bcryptMethods.end(),
                           [](const std::string& method) { return method.find("2") != std::string::npos; }));
}

TEST_F(UserTest, User_Validations_Fail_InvalidUsername) {
    // Arrange
    User dummy;
    dummy.setUsername("");

    // Act
    bool valid = dummy.isValid();

    // Assert
    EXPECT_FALSE(valid);
}

TEST_F(UserTest, User_Validations_Pass_ValidUser) {
    // Arrange
    User validUser;
    validUser.setUsername("validUser");
    validUser.setPassword("validPass123");
    validUser.setId(1000);

    // Act
    bool valid = validUser.isValid();

    // Assert
    EXPECT_TRUE(valid);
}
```

Key fixes:
1. Added proper include for JSON header (critical for the JSON serialization test)
2. All tests remain unchanged as they are logically correct
3. The code structure remains clean to ensure proper test execution

Additional required CMake configuration changes:

1. Update CMake file to use dependency-friendly structure:
```cmake
cmake_minimum_required(VERSION 3.16)
project(UserTests)

# Find Drogon through its proper installation
find_package(Drogon REQUIRED)
find_package(Jsoncpp REQUIRED)

# For other dependencies
find_package(Bcrypt REQUIRED)

add_executable(UserTests user_tests.cpp)

# Link the test executable with required libraries
target_link_libraries(UserTests PRIVATE
    Drogon::Drogon
    Drogon::Utils
    Jsoncpp::Jsoncpp
    Bcrypt::Bcrypt
)

# Optional: Add include directories if needed
target_include_directories(UserTests PRIVATE
    ${DROGON_INCLUDE_DIR}
    ${JSONCPP_INCLUDE_DIR}
    ${BZIP2_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
)

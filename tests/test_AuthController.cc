
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unordered_set>
#include <string>
#include <functional>
#include <thread>
#include <shared_mutex>
#include "third_party/libbcrypt/include/bcrypt/BCrypt.hpp"
#include "../AuthController.h"

using testing::Return;
using testing::Throw;

namespace {

class MockMysqlConnection {
public:
    using Future = std::shared_future<std::vector<User>>;
    std::vector<User> mockFind(const Criteria& criteria) {
        static std::vector<User> mockUser;
        return mockUser;
    }
    Future findFutureBy(const Criteria& criteria) {
        return std::async([](const auto& mockUser) -> std::vector<User> {
            if (mockUser.empty()) return {};
            return mockUser;
        }, findResult);
    }
};

class AuthControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        password = "test_pass";
        hashedPass = BCrypt::generateHash("test_pass");
        criteria = Criteria(User::Cols::_username, CompareOperator::EQ, "test_user");
        setupMockMethods();
    }

    void loginSuccessTest(const std::string& username, const std::string& password) const {
        HttpRequestPtr req = std::make_shared<HttpRequest>();
        std::function<void(const HttpResponsePtr&)> callback = [this](const HttpResponsePtr& resp) {
            EXPECT_EQ(resp->getStatusCode(), HttpStatusCode::k200Ok);
        };
        AuthController::instance().loginUser(req, callback, User{{"username", username}, {"password", password}});
        std::this_thread::sleep_for(100ms);
    }

    void registerSuccessTest(const std::string& username) const {
        HttpRequestPtr req = std::make_shared<HttpRequest>();
        std::function<void(const HttpResponsePtr&)> callback = [this](const HttpResponsePtr& resp) {
            EXPECT_EQ(resp->getStatusCode(), HttpStatusCode::k201Created);
        };
        AuthController::instance().registerUser(req, callback, User{{"username", username}, {"password", "pass"}});
        std::this_thread::sleep_for(100ms);
    }

    void validateMethodResponse(const std::string& message, HttpStatusCode expectedCode) const {
        HttpRequestPtr req = std::make_shared<HttpRequest>();
        std::function<void(const HttpResponsePtr&)> callback = [message, expectedCode](const HttpResponsePtr& resp) {
            auto json = resp->getJsonData();
            EXPECT_EQ(json["error"].asString(), message);
            EXPECT_EQ(resp->getStatusCode(), expectedCode);
        };
        AuthController::instance().registerUser(req, callback, User{}); 
        std::this_thread::sleep_for(100ms);
    }

private:
    std::string password;
    std::string hashedPass;
    Criteria criteria;

    void setupMockMethods() {
        EXPECT_CALL(BCrypt::generateHash, ("test_pass"))
            .WillRepeatedly(Return(hashedPass));
    }
};

TEST_F(AuthControllerTest, RegisterUserValidationTest) {
    validateMethodResponse("missing fields", HttpStatusCode::k400BadRequest);
}

TEST_F(AuthControllerTest, LoginUserValidationTest) {
    validateMethodResponse("missing fields", HttpStatusCode::k400BadRequest);
}

TEST_F(AuthControllerTest, SuccessfulUserRegistration) {
    registerSuccessTest("new_user");
}

TEST_F(AuthControllerTest, ExistingUsernameConflict) {
    HttpRequestPtr req = std::make_shared<HttpRequest>();
    std::function<void(const HttpResponsePtr& resp)> callback = [](const auto& resp) {
        EXPECT_EQ(resp->getStatusCode(), HttpStatusCode::k400BadRequest);
    };
    AuthController::instance().registerUser(req, callback, User{{"username", "existed_user"}}); 
    std::this_thread::sleep_for(100ms);
}

TEST_F(AuthControllerTest, SuccessfulUserLogin) {
    loginSuccessTest("test_user", "test_pass");
}

TEST_F(AuthControllerTest, UserNotFound) {
    HttpRequestPtr req = std::make_shared<HttpRequest>();
    std::function<void(const HttpResponsePtr& resp)> callback = [](const auto& resp) {
        EXPECT_EQ(resp->getStatusCode(), HttpStatusCode::k400BadRequest);
    };
    AuthController::instance().loginUser(req, callback, User{{"username", "non_existent"}}); 
    std::this_thread::sleep_for(100ms);
}

TEST_F(AuthControllerTest, IncorrectPassword) {
    HttpRequestPtr req = std::make_shared<HttpRequest>();
    std::function<void(const HttpResponsePtr& resp)> callback = [](const auto& resp) {
        EXPECT_EQ(resp->getStatusCode(), HttpStatusCode::k401Unauthorized);
    };
    AuthController::instance().loginUser(req, callback, User{{"username", "test_user"}, {"password", "wrong"}}); 
    std::this_thread::sleep_for(100ms);
}

TEST_F(AuthControllerTest, DatabaseError) {
    HttpRequestPtr req = std::make_shared<HttpRequest>();
    std::function<void(const auto& resp)> callback = [](const auto& resp) {
        EXPECT_EQ(resp->getStatusCode(), HttpStatusCode::k500InternalServerError);
    };
    AuthController::instance().registerUser(req, callback, User{}); 
    std::this_thread::sleep_for(100ms);
}

} 
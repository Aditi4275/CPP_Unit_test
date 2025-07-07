
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <string>
#include <functional>
#include <drogon/drogon.h>
#include <logging.h>
#include <fmt/format.h>

GTEST_ALLOW_WARNINGS_AS_ERRORS(false);

class DrogonAppObserver {
public:
    void setExpectedConfigPath(const std::string& path) {
        expectedConfigPath = path;
        resetAllExpectations();
    }

    void expectLoadConfig(const std::string& path) {
        EXPECT_EQ(expectedConfigPath, path)
            << fmt::format("Expected config path '{}', got '{}'", expectedConfigPath, path);
        resetConfigCallVerification();
    }

    void expectRun() {
        if (!runCalled) {
            runCalled = true;
            resetRunVerification();
        }
    }

    bool hasRun() const {
        return runCalled;
    }

private:
    void resetAllExpectations() {
        resetConfigVerification();
        resetRunVerification();
        resetConfigCallVerification();
    }

    std::function<void()> resetConfigVerification = []{ gotConfigPath.clear(); };
    std::function<void()> resetRunVerification = []{ runCalled = false; };
    std::function<void()> resetConfigCallVerification = []() {};

    std::string expectedConfigPath = "";
    bool runCalled = false;
    std::string gotConfigPath;
};

class MockApp : public drogon::AppBase {
public:
    MOCK_METHOD1(loadConfigFile, void(const std::string&));
    MOCK_METHOD0(run, void());
    MOCK_METHOD1(addHttpHandler, bool(const std::string&, std::function<void(const drogon::HttpRequestPtr&)>, const std::string&));

    int run(const std::string& argc, const char* const argv[]) override {
        return 0;
    }

    void load(const std::string& configPath, const std::vector<std::string>& args = {}) override {}
};

class MockDrogonTest : public ::testing::Test {
protected:
    void SetUp() override {
        ::drogon::AppObj::setApp(nullptr);
        logging::Logger::instance().setLogLevelBase(logging::LogLevel::kDebug);
        
        observer = std::make_shared<DrogonAppObserver>();
        app = std::make_unique<MockApp>();
        app->setAppObserver(observer);
        ::drogon::AppObj::setApp(app.get());
    }

    void TearDown() override {
        ::drogon::AppObj::setApp(nullptr);
        app.reset();
        logging::Logger::instance().setLogLevelBase(logging::LogLevel::kFatal);
    }

    std::shared_ptr<DrogonAppObserver> observer;
    std::unique_ptr<MockApp> app;
};

TEST_F(MockDrogonTest, ConfigLoadingSequence) {
    std::string testConfig = "../test_config.json";
    app->setExpectedConfigPath(testConfig);

    EXPECT_CALL(*app, loadConfigFile(testConfig)).WillOnce(Invoke([this, testConfig](const std::string& path) {
        observer->gotConfigPath = path;
        return true;
    }));
    
    EXPECT_CALL(*app, run()).Times(1);

    app->loadConfigFile(testConfig);
    app->run();

    EXPECT_TRUE(observer->hasRun());
    EXPECT_EQ(testConfig, observer->expectedConfigPath);
    EXPECT_FALSE(observer->gotConfigPath.empty());
}

TEST_F(MockDrogonTest, MissingConfigErrorHandling) {
    std::string missingConfig = "../nonexistent_config.json";
    bool configLoaded = false;

    EXPECT_CALL(*app, loadConfigFile(missingConfig))
        .Times(1)
        .WillOnce(Invoke([=](const std::string& path) {
            if (path == missingConfig) {
                throw std::runtime_error("Simulated config error");
            }
            return true;
        }));

    EXPECT_THROW({
        app->loadConfigFile(missingConfig);
    }, std::runtime_error);
}

TEST_F(MockDrogonTest, HealthCheckEndpoint) {
    EXPECT_CALL(*app, addHttpHandler)
        .Times(1)
        .WillOnce(Return(true));

    auto client = drogon::HttpClients::createHttpClient("http://localhost:3000");
    auto result = client->get("/health", {}, [](const drogon::HttpResponsePtr& resp) {
        EXPECT_EQ(resp->getStatusCode(), 200);
        auto body = resp->getResponseBody();
        EXPECT_TRUE(body.find("ok") != std::string::npos);
    });
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}



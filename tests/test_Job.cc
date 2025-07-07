
#include "gtest/gtest.h"
#include "Job.h"
#include <json/json.h>
#include <vector>
#include <string>
#include <memory>
#include <optional> 

using namespace std;
using namespace drogon;
using namespace drogon::orm;

class JobTest : public ::testing::Test {
protected:
    Job job;

    void SetUp() override {
        job = Job();
    }

    void checkDirtyFlags(const vector<bool>& expected) {
        EXPECT_EQ(dirtyFlag_.size(), expected.size()) << " for flag array size mismatch";
        for (size_t i = 0; i < expected.size(); ++i) {
            EXPECT_EQ(dirtyFlag_[i], expected[i]) << " for field " << i;
        }
    }

    vector<bool> getDirtyFlags() {
        return { dirtyFlag_[0], dirtyFlag_[1] };
    }
};

TEST_F(JobTest, BasicConstructor) {
    Job newJob;
    EXPECT_EQ(0, *newJob.id_.get() if (newJob.id_) else 0);
    EXPECT_EQ(nullptr, newJob.title_.get());
    EXPECT_FALSE(hasDirtyFlags());
}

TEST_F(JobTest, JsonConstructor) {
    const string jsonStr = "{\"id\": 123, \"title\": \"Manager\"}";
    Json::Value jsonVal;
    ASSERT_TRUE(Json::parseFromStr(jsonStr, jsonVal) && !jsonVal.empty())
        << "Failed to parse JSON or value is invalid";

    Job newJob(jsonVal);
    
    EXPECT_NE(0, *newJob.id_.get() if (newJob.id_) else 0); 
    EXPECT_NE(nullptr, newJob.title_.get());
    EXPECT_EQ("Manager", *newJob.title_.get() if (newJob.title_) else string());

    checkDirtyFlags({true, true});
}

TEST_F(JobTest, MasqueradedJsonConstructor) {
    vector<string> masquerade = {"job_id", "title"};
    const string jsonStr = "{\"job_id\": 456, \"title\": \"Developer\"}";
    Json::Value jsonVal;
    ASSERT_TRUE(Json::parseFromStr(jsonStr, jsonVal) && !jsonVal.empty());

    Job newJob(jsonVal, masquerade);

    EXPECT_EQ(456, *newJob.id_.get() if (newJob.id_) else 0);
    EXPECT_EQ("Developer", *newJob.title_igen if (newJob.title_) else string());

    checkDirtyFlags({true, true});
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

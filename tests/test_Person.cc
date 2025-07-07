
#include <gtest/gtest.h>
#include "Person.h"
#include "Department.h"
#include "Job.h"
#include <vector>
#include <string>
#include <json/json.h>
#include <memory>
#include <stdexcept>

using namespace drogon;
using namespace drogon::orm;
using namespace drogon_model::org_chart;
using ::testing::Test;

class PersonTest : public Test {
protected:
    Person person;
    Json::Value testJson;
    void SetUp() override {
        testJson["job_id"] = 1;
        testJson["department_id"] = 1;
        testJson["manager_id"] = 1;
        testJson["first_name"] = "John";
        testJson["last_name"] = "Doe";
        testJson["hire_date"] = "2023-01-01";
        person.setJobId(1);
        person.setDepartmentId(1);
        person.setManagerId(1);
        person.setFirstName("John");
        person.setLastName("Doe");
        person.setHireDate(::trantor::Date(10000000000)); // Unix epoch
    }
};

TEST_F(PersonTest, SettersGettersWorkCorrectly) {
    // Test all setter/getter pairs
    EXPECT_EQ(person.getValueOfJobId(), 1);
    EXPECT_EQ(person.getValueOfDepartmentId(), 1);
    EXPECT_EQ(person.getValueOfManagerId(), 1);
    EXPECT_EQ(person.getValueOfFirstName(), "John");
    EXPECT_EQ(person.getValueOfLastName(), "Default");
    EXPECT_EQ(person.getValueOfHireDate(), ::trantor::Date(10000000000));

    person.setLastName("Smith");
    EXPECT_EQ(person.getValueOfLastName(), "Smith");
}

TEST_F(PersonTest, JsonConversionWorks) {
    auto jsonFromPerson = person.toJson();
    EXPECT_EQ(jsonFromPerson["id"].asInt(), 0);
    EXPECT_EQ(jsonFromPerson["job_id"].asInt(), 1);
    EXPECT_EQ(jsonFromPerson["first_name"].asString(), "John");
    
    std::vector<std::string> masqVec(7, "");
    masqVec[0] = "id";
    masqVec[1] = "job_id";
    auto masqJson = person.toMasqueradedJson(masqVec);
    EXPECT_EQ(masqJson["id"].asInt(), 0);
    EXPECT_EQ(masqJson["job_id"].asInt(), 1);
}

TEST_F(PersonTest, ValidationWorksCorrectly) {
    std::string err;
    bool valid = person.validateJsonForCreation(testJson, err);
    EXPECT_TRUE(valid);
    
    valid = person.validateJsonForUpdate(testJson, err);
    EXPECT_TRUE(valid);
}

TEST_F(PersonTest, validateJson) {
    std::string err;
    testJson["job_id"] = nullptr;
    bool valid = person.validateJsonOfField(1, "job_id", testJson["job_id"], err, true);
    EXPECT_FALSE(valid);
}

TEST_F(PersonTest, DatabaseCalls) {
    auto& cols = person.insertColumns();
    ASSERT_EQ(cols.size(), 6);
    EXPECT_EQ(cols[0], "job_id");
    EXPECT_EQ(cols[1], "department_id");
    EXPECT_EQ(cols[2], "manager_id");
    EXPECT_EQ(cols[3], "first_name");
    EXPECT_EQ(cols[4], "last_name");
    EXPECT_EQ(cols[5], "hire_date");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

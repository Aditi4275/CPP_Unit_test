
#include <gtest/gtest.h>
#include "Department.h"
#include "Person.h"
#include <json/json.h>
#include <string>
#include <vector>
#include <memory>

using namespace std;
using namespace drogon::orm;
using namespace drogon::ctx;

class DepartmentTest : public ::testing::Test {
protected:
    Department dept;
    Department deptRow;
    Department deptJson;

    void SetUp() override {
        
    }

    void TearDown() override {
        
    }
};

TEST_F(DepartmentTest, Constructor_Default) {
    EXPECT_EQ(dept.getId(), nullptr);
    EXPECT_EQ(dept.getName(), nullptr);
}

TEST_F(DepartmentTest, Constructor_FromRow_WithoutOffset) {
    using namespace drogon::orm;
    vector<Row> mockRow;
    Row row;
    row["id"] = 1;
    row["name"] = "Engineering";
    mockRow.push_back(row);

    Department d(mockRow[0]);
    EXPECT_EQ(d.getId(), make_shared<int32_t>(1));
    EXPECT_EQ(d.getName(), make_shared<string>("Engineering"));
}

TEST_F(DepartmentTest, Constructor_FromRow_WithOffset) {
    vector<Row> mockRow;
    Row row;
    row["id"] = 1;
    row["name"] = "Marketing";
    mockRow.push_back(row);

    Department d(mockRow, 0);
    EXPECT_EQ(d.getId(), make_shared<int32_t>(1));
    EXPECT_EQ(d.getName(), make_shared<string>("Marketing"));
}

TEST_F(DepartmentTest, Constructor_FromJson_Normal) {
    Json::Value json;
    json["id"] = 100;
    json["name"] = "Finance";
    deptJson = Department(json);
    EXPECT_EQ(deptJson.getId(), make_shared<int32_t>(100));
    EXPECT_EQ(deptJson.getName(), make_shared<string>("Finance"));
}

TEST_F(DepartmentTest, Constructor_FromJson_Masqueraded) {
    Json::Value json;
    json["dept_id"] = 200;
    json["dept_name"] = "Sales";
    deptJson = Department(json, {"dept_id", "dept_name"});
    EXPECT_EQ(deptJson.getId(), make_shared<int32_t>(200));
    EXPECT_EQ(deptJson.getName(), make_shared<string>("Sales"));
}

TEST_F(DepartmentTest, SettersAndGetters) {
    dept.setId(1);
    dept.setName("HR");
    EXPECT_EQ(dept.getId(), make_shared<int32_t>(1));
    EXPECT_EQ(dept.getName(), make_shared<string>("HR"));
}

TEST_F(DepartmentTest, Validation_Creation) {
    string error;
    bool valid = dept.validateJsonForCreation(Json::nullValue, error);
    EXPECT_FALSE(valid);
    EXPECT_EQ(error, "The id column cannot be null");
}

TEST_F(DepartmentTest, JSON_Conversion) {
    dept.setId(1);
    dept.setName("IT");
    Json::Value json = dept.toJson();
    EXPECT_TRUE(json.isMember("id"));
    EXPECT_TRUE(json.isMember("name"));
    EXPECT_EQ(json["id"].asInt(), 1);
    EXPECT_EQ(json["name"].asString(), "IT");
}

TEST_F(DepartmentTest, Update_Methods) {
    dept.updateByJson(Json::Value("id").set("_nodoc_:1").asObject());
    dept.updateByMasqueradedJson(Json::Value("id").set("_nodoc_:2").asObject(), {"id"});
}

TEST_F(DepartmentTest, GetPersons) {
    EXPECT_NO_THROW(dept.getPersons(nullptr, [](const vector<Person>& p) {}, nullptr));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}



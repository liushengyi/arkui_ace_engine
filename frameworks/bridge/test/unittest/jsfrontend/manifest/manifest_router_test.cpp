/*
* Copyright (c) 2022 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <memory>

#include "gtest/gtest.h"

#include "base/memory/ace_type.h"
#include "frameworks/bridge/common/manifest/manifest_router.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::Framework {
namespace {

const std::string PAGES_LIST = "                              "
                               "{                             "
                               "  \"pages\": [                "
                               "                \"index\",    "
                               "                \"first\"     "
                               "              ]               "
                               "}";

const std::string INDEX_JS = "index.js";
const std::string FIRST_JS = "first.js";
const std::string SECOND_JS = "second.js";
} // namespace

class ManifestRouterTest : public testing::Test {
public:
   static void SetUpTestCase();
   static void TearDownTestCase();
   void SetUp();
   void TearDown();
};

void ManifestRouterTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "ManifestRouterTest SetUpTestCase";
}

void ManifestRouterTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "ManifestRouterTest TearDownTestCase";
}

void ManifestRouterTest::SetUp()
{
    GTEST_LOG_(INFO) << "ManifestRouterTest SetUp";
}

void ManifestRouterTest::TearDown()
{
    GTEST_LOG_(INFO) << "ManifestRouterTest TearDown";
}

/**
* @tc.name: ManifestRouterTest001
* @tc.desc: ManifestRouter get path with empty uri or empty pages_
* @tc.type: FUNC
*/
HWTEST_F(ManifestRouterTest, ManifestRouterTest001, TestSize.Level1)
{
    std::string uri;
    auto manifestRouter = AceType::MakeRefPtr<ManifestRouter>();
    auto result = manifestRouter->GetPagePath(uri);
    ASSERT_EQ(result, "");

    uri = "aaa";
    auto pageList = manifestRouter->GetPageList();
    pageList.clear();
    result = manifestRouter->GetPagePath(uri);
    ASSERT_EQ(result, "");
}

/**
* @tc.name: ManifestRouterTest002
* @tc.desc: ManifestRouter get path with root uri or correct uri
* @tc.type: FUNC
 */
HWTEST_F(ManifestRouterTest, ManifestRouterTest002, TestSize.Level1)
{
    auto manifestRouter = AceType::MakeRefPtr<ManifestRouter>();
    auto rootJson = JsonUtil::ParseJsonString(PAGES_LIST);
    manifestRouter->RouterParse(rootJson);

    std::string uri = "/";
    auto result = manifestRouter->GetPagePath(uri);
    ASSERT_EQ(result, INDEX_JS);

    uri = "aaa";
    result = manifestRouter->GetPagePath(uri);
    ASSERT_EQ(result, "");

    uri = "first";
    result = manifestRouter->GetPagePath(uri);
    ASSERT_EQ(result, FIRST_JS);
}

/**
* @tc.name: ManifestRouterTest003
* @tc.desc: ManifestRouter get path with empty uri or uri which contains correct suffix
* @tc.type: FUNC
*/
HWTEST_F(ManifestRouterTest, ManifestRouterTest003, TestSize.Level1)
{
    std::string uri;
    auto manifestRouter = AceType::MakeRefPtr<ManifestRouter>();
    auto result = manifestRouter->GetPagePath(uri);
    auto entry = manifestRouter->GetEntry(".js");
    ASSERT_EQ(result, "");
    ASSERT_EQ(entry, "");

    auto rootJson = JsonUtil::ParseJsonString(PAGES_LIST);
    manifestRouter->RouterParse(rootJson);
    uri = "/";
    result = manifestRouter->GetPagePath(uri, ".js");
    entry = manifestRouter->GetEntry(".js");
    ASSERT_EQ(result, INDEX_JS);
    ASSERT_EQ(entry, INDEX_JS);

    uri = "/etc";
    result = manifestRouter->GetPagePath(uri, ".js");
    ASSERT_EQ(result, "");

    uri = "first";
    result = manifestRouter->GetPagePath(uri, ".js");
    ASSERT_EQ(result, FIRST_JS);

    uri = "second.js";
    result = manifestRouter->GetPagePath(uri, ".js");
    ASSERT_EQ(result, SECOND_JS);
}
} // namespace OHOS::Ace
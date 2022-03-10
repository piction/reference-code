#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>

#include "log.h"
#include "topicHandler.h"

TEST(topicHandler,basics ){
    Log::Init();
    std::vector<std::string> subribeTopics = {"test/topic","testTopic/#","aaa/+/bbb","+/c/bb/+/ff" };
    auto sut = TopicHandler(subribeTopics);

    EXPECT_TRUE(sut.isTopicValidForHandling("test/topic"));
    EXPECT_FALSE(sut.isTopicValidForHandling("test/topc"));
    EXPECT_FALSE(sut.isTopicValidForHandling("test/topic/hello"));
    EXPECT_FALSE(sut.isTopicValidForHandling("test/topicxx"));


    EXPECT_TRUE(sut.isTopicValidForHandling("testTopic/boguse"));
    EXPECT_TRUE(sut.isTopicValidForHandling("testTopic/boguse/adsf"));
    EXPECT_TRUE(sut.isTopicValidForHandling("testTopic/boguse/adsf/adsf/asdf"));
    EXPECT_FALSE(sut.isTopicValidForHandling("testTopicx/#"));

    EXPECT_TRUE(sut.isTopicValidForHandling("aaa/test/bbb"));
    EXPECT_TRUE(sut.isTopicValidForHandling("aaa/1/bbb"));
    EXPECT_FALSE(sut.isTopicValidForHandling("aaa/1/cc/bbb"));
  

    EXPECT_TRUE(sut.isTopicValidForHandling("x/c/bb/x/ff"));
    EXPECT_TRUE(sut.isTopicValidForHandling("xxx/c/bb/xxx/ff"));
    EXPECT_FALSE(sut.isTopicValidForHandling("xxx/c/bbx/xxx/ff"));

    EXPECT_TRUE(true);
}
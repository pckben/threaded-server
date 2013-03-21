#include "ConfigTest.h"
#include <string>
#include <fstream>

#include "Config.h"

using namespace std;
using namespace speechsvr;

void WriteConfig(string file_name, string text) {
  ofstream ofs(file_name.c_str());
  ofs << text;
  ofs.close();
}

TEST_F(ConfigTest, Constructor) {
  WriteConfig("temp", "key = value");
  Config conf("temp");
}

TEST_F(ConfigTest, Size) {
  WriteConfig("temp", "1 = a\n2 = b\n3 = c\n4 = d");
  Config conf("temp");
  EXPECT_EQ(4, conf.Size());
}

TEST_F(ConfigTest, WhiteSpaces) {
  WriteConfig("temp", "  \taaa=bbb\t\t\t   \t\t \r");
  Config conf("temp");
  EXPECT_STREQ("bbb", conf.GetString("aaa").c_str());

  WriteConfig("temp", "ccc\t\t \t     =\t  \tddd");
  Config conf1("temp");
  EXPECT_STREQ("ddd", conf1.GetString("ccc").c_str());
}

TEST_F(ConfigTest, EmptyLine) {
  WriteConfig("temp", "\n\na=b\n\n\n\n\t\t     \n   \n\t\nc=d\n\n");
  Config conf("temp");
  EXPECT_STREQ("b", conf.GetString("a").c_str());
  EXPECT_STREQ("d", conf.GetString("c").c_str());
}

TEST_F(ConfigTest, CommentLine) {
  WriteConfig("temp", "a=b\n#c=d\ne=f");
  Config conf("temp");
  EXPECT_EQ(2, conf.Size());
  EXPECT_STREQ("b", conf.GetString("a").c_str());
  EXPECT_STREQ("f", conf.GetString("e").c_str());

  WriteConfig("temp", "a=b\n  \t\t  ###===###===\t  \ne=f");
  Config conf1("temp");
  EXPECT_EQ(2, conf1.Size());
}

TEST_F(ConfigTest, Get) {
  WriteConfig("temp", "a = -123\nb = -0.555\nc = 1e-5");
  Config conf("temp");
  EXPECT_EQ(-123, conf.GetInt("a"));
  EXPECT_FLOAT_EQ(-0.555, conf.GetFloat("b"));
  EXPECT_FLOAT_EQ(1e-5, conf.GetFloat("c"));

  WriteConfig("temp", "some_long_key_name11111111 = path/to/some/dir/file.ext");
  Config conf1("temp");
  EXPECT_STREQ("path/to/some/dir/file.ext",
      conf1.GetString("some_long_key_name11111111").c_str());

  WriteConfig("temp", " key = value with spaces    ");
  Config conf2("temp");
  EXPECT_STREQ("value with spaces", conf2.GetString("key").c_str());
}

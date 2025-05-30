#include <gtest/gtest.h>
#include <string>
using namespace std;

int main(int argc, const char **argv)
{
  // implement some niceties from the old UnitTest++ days
  const char listTests[]="--gtest_list_tests";
  string pattern;
  
  if (argc>1)
    if (string(argv[1])=="-l")
      argv[1]=listTests;
    else if (argv[1][0]!='-') // no option provided, assume it is a pattern
      {
        pattern=string("--gtest_filter=")+argv[1];
        argv[1]=pattern.c_str();
      }
  ::testing::InitGoogleTest(&argc, const_cast<char**>(argv));
  return RUN_ALL_TESTS();
}

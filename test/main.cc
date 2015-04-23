/*
  @copyright Russell Standish 2012
  @author Russell Standish

  Open source licensed under the MIT license.

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <UnitTest++/UnitTest++.h>
#include <UnitTest++/TestReporterStdout.h>
#include <boost/regex.hpp>
using namespace UnitTest;
using std::string;
using std::cout;
using std::endl;

namespace
{
  string name(const Test& t)
  {return string(t.m_details.suiteName)+"::"+t.m_details.testName;}

  struct RegExPredicate
  {
    boost::regex exp;
    RegExPredicate(): exp(".*") {} // equivalent to true
    RegExPredicate(const string& exp): exp(exp) {}
    bool operator()(const Test* const test) const
    {return boost::regex_match(name(*test), exp);}
  };
}

int main(int argc, const char** argv)
{
  if (argc>1)
    {
      bool listTests=false;
      // handle options
      if (argv[1][0]=='-')
        {
          switch (argv[1][1])
            {
            case 'h': case '?':
              cout << "Usage "<<argv[0]<<" [-h] [-l] [regex]\n";
              cout << " -h: print this help message\n";
              cout << " -l: list all tests matching given regex\n";
              cout << " no option: run all tests matching given pattern\n";
              cout << " no pattern: = all tests, as if .* was provided as the pattern\n";
              return 0;
            case 'l':
              listTests=true;
              break;
            }
          argv++;
          argc--;
        }
      RegExPredicate exp;
      if (argc>1)
        exp=RegExPredicate(argv[1]);
      if (listTests)
        {
          for (Test* t=Test::GetTestList().GetHead(); t; t=t->next)
            if (exp(t))
              cout << name(*t)<<endl;
          return 0;
        }
      else
        {
          TestReporterStdout reporter;
          return TestRunner(reporter).RunTestsIf(Test::GetTestList(), NULL, exp, 0);
        }
    }  
      
  return RunAllTests();
}

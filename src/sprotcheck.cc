/*
 * Copyright 2009 Bert van der Weerd <bert@superstring.nl>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "sprot.h"

#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TextTestProgressListener.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <stdexcept>

class UnitTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(UnitTest);

  CPPUNIT_TEST(test1);
  CPPUNIT_TEST(test2);

  CPPUNIT_TEST_SUITE_END();

private:
  sprot* prog;
  std::vector<std::string> args;

public:
  virtual void setUp () 
  {
    args.push_back("a");
    args.push_back("b");
    prog = new sprot(args);
  }
  virtual void tearDown() 
  {
    delete prog;
  }
  
  void test1() 
  {
    int a = 10;
    int b(10);
    CPPUNIT_ASSERT( a == b );
  }
  
  void test2() 
  {
    float tol = 0.05;
    float a = 10.;
    float b = a + tol/10.;
    CPPUNIT_ASSERT_DOUBLES_EQUAL( a, b, tol );
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(UnitTest);
 
int 
main( int argc, char* argv[] )
{
  // Create the event manager and test controller
  CppUnit::TestResult controller;
  // Add a listener that colllects test result
  CppUnit::TestResultCollector result;
   // Add a listener that print dots as test run.
  CppUnit::TextTestProgressListener progress;

  controller.addListener(&result);        
  controller.addListener(&progress);      
 
  // Add the top suite to the test runner
  CppUnit::TestRunner runner;
  runner.addTest( CppUnit::TestFactoryRegistry::getRegistry().makeTest() );   

 
  try
    {
      std::string testPath = (argc > 1) ? std::string(argv[1]) : "";

      runner.run( controller, testPath );
 
      // Print test in a compiler compatible format.
      CppUnit::CompilerOutputter outputter( &result, std::cout );
      outputter.write();                      
    }
  catch ( std::invalid_argument &e )  // Test path not resolved
    {
      std::cerr << "Exception: " << e.what() 
                << " (std::invalid_argument)" << std::endl;
      return 0;
    }
 

  return result.wasSuccessful() ? 0 : 1;
}


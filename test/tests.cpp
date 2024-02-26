#include <catch2/catch_test_macros.hpp>


#include <scoped_new/sample_library.hpp>
#include <scoped_new/scoped_new.h>

#include <iostream>
#include <string>

using namespace std;

namespace
{
    class Lifetime
    {
      inline static int s_count = 0;
      int m_num = -1;
      string m_name;
      
    public:
      Lifetime()
      {
        std::cout << " -> " << ++s_count << "\n";
      }

      explicit Lifetime(int num)
      : m_num(num)
      {
        std::cout << " -> " << num << " " << ++s_count << "\n";
      }  

      Lifetime(int num, const string& name)
      : m_num(num)
      , m_name(name)
      {
        std::cout << " -> " << num << " " << name << " " << ++s_count << "\n";
      }  
      
      ~Lifetime()
      {
        std::cout << " <- " << m_num << " " << m_name << " " << s_count-- << "\n";
      }
      
      // make sure it works with non-moveable types
      Lifetime(const Lifetime&) = delete;
      Lifetime& operator=(const Lifetime&) = delete;
      Lifetime(Lifetime&&) = delete;
      Lifetime& operator=(Lifetime&&) = delete;

    };
    
}

TEST_CASE("Factorials are computed", "[factorial]")
{
  REQUIRE(factorial(0) == 1);
  REQUIRE(factorial(1) == 1);
  REQUIRE(factorial(2) == 2);
  REQUIRE(factorial(3) == 6);
  //REQUIRE(factorial(10) == 3628800);
}

TEST_CASE("XXXXX")
{
  {
    scope::scoped_new<Lifetime> lt;
    for(int i = 0; i < 121; i++)
    {
      lt.make();
      lt.make(i);
      lt.make(i, "fred");
    }
  }  
}

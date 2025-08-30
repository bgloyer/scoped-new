#include <catch2/catch_test_macros.hpp>


#include <scoped_new/sample_library.hpp>
#include <scoped_new/filo.h>
#include <scoped_new/scoped_new.h>

#include <iostream>
#include <string>

using namespace std;

namespace
{
    class NonMoveable
    {
      inline static int s_count = 0;
      int m_num = -1;
      string m_name;
      
    public:
      NonMoveable()
      {
        std::cout << " -> " << ++s_count << "\n";
      }
      
      explicit NonMoveable(int num)
      : m_num(num)
      {
        std::cout << " -> " << num << " " << ++s_count << "\n";
      }  

      NonMoveable(int num, const string& name)
      : m_num(num)
      , m_name(name)
      {
        std::cout << " -> " << num << " " << name << " " << ++s_count << "\n";
      }  
      
      ~NonMoveable()
      {
        std::cout << " <- " << m_num << " " << m_name << " " << s_count-- << "\n";
      }
      
      // make sure it works with non-moveable types
      NonMoveable(const NonMoveable&) = delete;
      NonMoveable& operator=(const NonMoveable&) = delete;
      NonMoveable(NonMoveable&&) = delete;
      NonMoveable& operator=(NonMoveable&&) = delete;
    };

    class MoveableOnly
    {
      inline static int s_count = 0;
      int m_num = -1;
      string m_name;
      
    public:
      MoveableOnly()
      {
        std::cout << " -> " << ++s_count << "\n";
      }
      
      explicit MoveableOnly(int num)
      : m_num(num)
      {
        std::cout << " -> " << num << " " << ++s_count << "\n";
      }  

      MoveableOnly(int num, const string& name)
      : m_num(num)
      , m_name(name)
      {
        std::cout << " -> " << num << " " << name << " " << ++s_count << "\n";
      }  
      
      ~MoveableOnly()
      {
        std::cout << " <- " << m_num << " " << m_name << " " << s_count-- << "\n";
      }
      
      // make sure it works with non-moveable types
      MoveableOnly(const MoveableOnly&) = delete;
      MoveableOnly& operator=(const MoveableOnly&) = delete;
      MoveableOnly(MoveableOnly&&) = default;
      MoveableOnly& operator=(MoveableOnly&&) = default;
    };

    class CopyableOnly
    {
      inline static int s_count = 0;
      int m_num = -1;
      string m_name;
      
    public:
      CopyableOnly()
      {
        std::cout << " -> " << ++s_count << "\n";
      }
      
      explicit CopyableOnly(int num)
      : m_num(num)
      {
        std::cout << " -> " << num << " " << ++s_count << "\n";
      }  

      CopyableOnly(int num, const string& name)
      : m_num(num)
      , m_name(name)
      {
        std::cout << " -> " << num << " " << name << " " << ++s_count << "\n";
      }  
      
      ~CopyableOnly()
      {
        std::cout << " <- " << m_num << " " << m_name << " " << s_count-- << "\n";
      }
      
      // make sure it works with non-moveable types
      CopyableOnly(const CopyableOnly&) = default;
      CopyableOnly& operator=(const CopyableOnly&) = default;
      CopyableOnly(CopyableOnly&&) = delete;
      CopyableOnly& operator=(CopyableOnly&&) = delete;
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

TEST_CASE("filo")
{
  {
    scope::filo<NonMoveable> lt;
    for(int i = 0; i < 3; i++)
    {
      lt.emplace();
      lt.emplace(i);
      lt.emplace(i, "fred");
    }
  }  
}

TEST_CASE("scoped_new")
{
  {
    scope::scoped_new sn;
    {
      sn.emplace<NonMoveable>();
      sn.emplace<NonMoveable>(4);
      sn.emplace<NonMoveable>(4, "bob");
      sn.insert(MoveableOnly{5, "george"});
      MoveableOnly moveableOnly(6, "alice");
      sn.insert(std::move(moveableOnly));
            
      const CopyableOnly copyableOnly(7, "lisa");
      sn.insert(copyableOnly);
    }
  }  
}

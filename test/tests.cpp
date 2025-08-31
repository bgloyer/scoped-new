#include <catch2/catch_test_macros.hpp>

#include <scoped_new/sample_library.hpp>
#include <scoped_new/filo.h>
#include <scoped_new/scoped_new.h>

#include <iostream>
#include <memory>
#include <print>
#include <string>
#include <vector>

using namespace std;

namespace
{
  using ObjectLifetimeVerifier = shared_ptr<nullptr_t>; // uses custom deleter for verification

  class LiftimeTracker
  {
    ObjectLifetimeVerifier m_objectZero;
    weak_ptr<nullptr_t> m_lastCreatedObj;
    vector<weak_ptr<nullptr_t>> m_allObjects;

  public:

    LiftimeTracker()
    {
      // create a first object
      m_objectZero = shared_ptr<nullptr_t>(nullptr, [](nullptr_t){});
      m_lastCreatedObj = m_objectZero;
    }

    ~LiftimeTracker()
    {
      VerifyNoneActive();
    }

    ObjectLifetimeVerifier CreateVerifier()
    {
      static int id = 0;
      std::println("constructing {}", id);

      // uses the shared_ptr/weak_ptr relationship to verify lifetimes
      auto deleteVerify = [lastObj = m_lastCreatedObj, id = id](nullptr_t)
      {
        // since object are destroyed in the reverse order of creation, all
        // objects created before this one should not be expired.  check the
        // one object created before this one which recursively shows they are
        // all valid
        std::println("dv {}", id);
        REQUIRE(!lastObj.expired());
      };
      id++;
      
      shared_ptr<nullptr_t> obj(nullptr, deleteVerify);
      m_lastCreatedObj = obj;
      m_allObjects.push_back(m_lastCreatedObj);

      return obj;
    }

  private:

    void VerifyNoneActive()
    {
      std::println("active?");
      // at this point, all objects should be destroyed and the weak_pts's expired
      for(auto wp : m_allObjects)
      {
        std::print(".");
        REQUIRE(wp.expired());
      }
    }
  };

    class NonMoveable
    {
      ObjectLifetimeVerifier m_verify;

    public:
      explicit NonMoveable(LiftimeTracker &tracker)
      : m_verify(tracker.CreateVerifier())
      {
      }

      ~NonMoveable() = default;

      // make sure it works with non-moveable types
      NonMoveable(const NonMoveable&) = delete;
      NonMoveable& operator=(const NonMoveable&) = delete;
      NonMoveable(NonMoveable&&) = delete;
      NonMoveable& operator=(NonMoveable&&) = delete;
    };

    class MoveableOnly
    {
      ObjectLifetimeVerifier m_verify;

    public:
      explicit MoveableOnly(LiftimeTracker &tracker)
      : m_verify(tracker.CreateVerifier())
      {
      }

      ~MoveableOnly() = default;

      // make sure it works with moveable non-copyable  types
      MoveableOnly(const MoveableOnly&) = delete;
      MoveableOnly& operator=(const MoveableOnly&) = delete;
      MoveableOnly(MoveableOnly&&) = default;
      MoveableOnly& operator=(MoveableOnly&&) = default;
    };

    class CopyableOnly
    {
      ObjectLifetimeVerifier m_verify;

    public:
      explicit CopyableOnly(LiftimeTracker &tracker)
      : m_verify(tracker.CreateVerifier())
      {
      }

      ~CopyableOnly() = default;

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
    LiftimeTracker tracker;
    scope::filo<NonMoveable> lt;
    for(int i = 0; i < 3; i++)
    {
      lt.emplace(tracker);
    }
  }  
}

TEST_CASE("scoped_new")
{
  {
    LiftimeTracker tracker;
    scope::scoped_new sn;
    {
      sn.emplace<NonMoveable>(tracker);
      sn.insert(MoveableOnly{tracker});
      MoveableOnly moveableOnly(tracker);
      sn.insert(std::move(moveableOnly));
            
      const CopyableOnly copyableOnly(tracker);
      sn.insert(copyableOnly);
    }
  }  
}

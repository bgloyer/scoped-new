#pragma once

#include<memory>

namespace scope
{
  class scoped_new
  {
    struct deleter_list
    {
      deleter_list *next_deleter;
      virtual void destroy() = 0;
      virtual ~deleter_list() = default;
    };

    template<class T>
    struct T_deleter : public deleter_list
    {
      char spot[sizeof(T)];

      void link(deleter_list*& item) noexcept
      {
        next_deleter = item;
        item = this;
      }
      
      void destroy() override
      {
        reinterpret_cast<T&>(spot).~T();
      }
    };
  
    deleter_list* m_next = nullptr;
    
  public:

    scoped_new() = default;
    scoped_new(const scoped_new&) = delete;
    scoped_new& operator=(const scoped_new&) = delete;
    scoped_new(scoped_new&&) = default;
    scoped_new& operator=(scoped_new&&) = default;
    
    ~scoped_new()
    {
      auto* ptr = m_next;
      while(ptr)
      {
        auto next = ptr->next_deleter;
        ptr->destroy();
        delete ptr;
        ptr = next;
      }
    }
    
    template<typename T, typename ...Params>
    T* emplace(Params&&... params)
    {     
      auto item = std::make_unique<T_deleter<T>>();
      auto* object = new (&item->spot) T(std::forward<Params>(params)...);
      item->link(m_next);
      item.release();
      return object;
    }

    template<typename T>
    T* emplace(T&& t)
    {     
      auto item = std::make_unique<T_deleter<T>>();
      auto* object = new (&item->spot) T(std::move(t));
      item->link(m_next);
      item.release();
      return object;
    }

    template<typename T>
    T* emplace(const T& t)
    {     
      auto item = std::make_unique<T_deleter<T>>();
      auto* object = new (&item->spot) T(t);
      item->link(m_next);
      item.release();
      return object;
    }
  };
}

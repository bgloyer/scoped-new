#pragma once

#include<memory>

namespace scope
{
  class scoped_new
  {
    class deleter_list
    {
      deleter_list *m_next_deleter;

    public:
      constexpr explicit deleter_list(deleter_list *head) noexcept
      : m_next_deleter(head) {}
      
//      void insert(deleter_list*& item) noexcept
//      {
//        m_next_deleter = item;
//        item = this;
//      }
      
      virtual void destroy() = 0;
      virtual ~deleter_list() = default;
      
      static void destroy_all(deleter_list *list)
      {
        while(list)
        {
          auto next = list->m_next_deleter;
          list->destroy();
          delete list;
          list = next;
        }
      };
    };

    template<class T>
    struct T_deleter : public deleter_list
    {
      explicit constexpr T_deleter(deleter_list *head) noexcept
      : deleter_list(head) {}
      
      char spot[sizeof(T)];
      
      void destroy() override
      {
        reinterpret_cast<T&>(spot).~T();
      }
    };
  
    deleter_list* m_head = nullptr;
    
  public:

    scoped_new() = default;
    scoped_new(const scoped_new&) = delete;
    scoped_new& operator=(const scoped_new&) = delete;
    scoped_new(scoped_new&&) = default;
    scoped_new& operator=(scoped_new&&) = default;
    
    ~scoped_new()
    {
      deleter_list::destroy_all(m_head);
    }
    
    template<typename T, typename ...Params>
    T* emplace(Params&&... params)
    {     
      auto item = std::make_unique<T_deleter<T>>(m_head);
      auto* object = new (&item->spot) T(std::forward<Params>(params)...);
    //  item->insert(m_next);
      m_head = item.get();
      item.release();
      return object;
    }

    template<typename T>
    T* emplace(T&& t)
    {     
      auto item = std::make_unique<T_deleter<T>>(m_head);
      auto* object = new (&item->spot) T(std::move(t));
//      item->insert(m_next);
      m_head = item.get();
      item.release();
      return object;
    }

    template<typename T>
    T* emplace(const T& t)
    {     
      auto item = std::make_unique<T_deleter<T>>(m_head);
      auto* object = new (&item->spot) T(t);
//      item->insert(m_next);
      m_head = item.get();
      item.release();
      return object;
    }
  };
}

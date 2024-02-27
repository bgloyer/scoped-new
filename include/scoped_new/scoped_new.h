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
    
    deleter_list* m_next = nullptr;
    
  public:

    scoped_new() = default;
    scoped_new(scoped_new&&) = default;
    scoped_new& operator=(scoped_new&&) = default;
    
    ~scoped_new()
    {
      auto ptr = m_next;
      while(ptr)
      {
        auto next = ptr->next_deleter;
        ptr->destroy();
        delete ptr;
        ptr = next;
      }
    }
    
    template<typename T, typename ...Params>
    T& emplace(Params&&... params)
    {
      struct T_deleter : public deleter_list
      {
        explicit T_deleter(deleter_list *next)
        {
          this->next_deleter = next;
        }
                
        char spot[sizeof(T)];
        void destroy() override
        {
          reinterpret_cast<T&>(spot).~T();
        }
      };
      
     auto* item = new T_deleter(m_next);
     m_next = item;
     T* buff = reinterpret_cast<T*>(item->spot);
     return *(new (buff) T(std::forward<Params>(params)...));
    }
  };
}


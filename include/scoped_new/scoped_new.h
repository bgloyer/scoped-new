#pragma once

#include<memory>

namespace scope
{
  template<typename T>
  class scoped_new
  {
    static constexpr int Width = 60;
    struct Buffer
    {
      std::unique_ptr<Buffer> next;
      void* slots[sizeof(T[Width])];
    };
    
    std::unique_ptr<Buffer> m_head;
    int m_index = Width;
    
  public:

    scoped_new() = default;
    scoped_new(scoped_new&&) = default;
    scoped_new& operator=(scoped_new&&) = default;
    
    ~scoped_new()
    {
      while(m_head)
      {
        T* buff = reinterpret_cast<T*>(m_head->slots);
        for(auto i = m_index - 1; i >= 0; --i)
        {
          buff[i].~T();
        }
        m_index = Width;
        auto rest = std::move(m_head->next);
        m_head = std::move(rest);
      }
    }
    
    template<typename ...Params>
    T& make(Params&&... params)
    {
      if(m_index >= Width)
      {
        auto b = std::make_unique<Buffer>();
        b->next = std::move(m_head);
        m_head = std::move(b);
        m_index = 0;
      }

      T* buff = reinterpret_cast<T*>(m_head->slots);
      return *(new (&buff[m_index++]) T(std::forward<Params>(params)...));
    }
  };
}


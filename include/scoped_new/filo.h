#pragma once

#include<memory>

namespace scope
{
  template<typename T>
  class filo
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

    filo() = default;
    filo(filo&&) = default;
    filo& operator=(filo&&) = default;
    
    ~filo()
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
    
    template<typename ...Args>
    T* emplace(Args&&... args)
    {
      if(m_index >= Width)
      {
        auto b = std::make_unique<Buffer>();
        b->next = std::move(m_head);
        m_head = std::move(b);
        m_index = 0;
      }

      T* buff = reinterpret_cast<T*>(m_head->slots);
      return new (&buff[m_index++]) T(std::forward<Args>(args)...);
    }
  };
}


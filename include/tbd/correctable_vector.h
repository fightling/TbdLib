#pragma once

#include <vector>

namespace tbd
{
  template<typename T>
  struct CorrectableVector :
    private std::vector<T>
  {
    typedef std::vector<T> container_type;
    typedef CorrectableVector<T> type;

    using container_type::empty;
    using container_type::size;

    typedef typename container_type::iterator iterator;
    typedef typename container_type::const_iterator const_iterator;
    typedef typename container_type::value_type value_type;
    typedef typename container_type::size_type size_type;

    CorrectableVector(bool _correct = false) : 
      correct_(_correct) {}


    value_type& operator[](size_type i)
    {
      correct_ = false;
      return container_type::operator[](i);
    }

    const value_type& operator[](size_type i) const
    {
      return container_type::operator[](i);
    }

    value_type& at(size_type i)
    {
      correct_ = false;
      return container_type::at(i);
    }

    const value_type& at(size_type i) const
    {
      return container_type::at(i);
    }

    const_iterator begin() const
    {
      return container_type::begin();
    }

    const_iterator end() const
    {
      return container_type::end();
    }

    template<typename ITERATOR> 
    void assign(ITERATOR _from, ITERATOR _to) 
    {
      correct_ = false;
      return container_type::assign(_from,_to);
    }

    value_type& back()
    {
      correct_ = false;
      return container_type::back();
    }

    const value_type& back() const
    {
      correct_ = false;
      return container_type::back();
    }

    value_type& front()
    {
      correct_ = false;
      return container_type::front();
    }

    const value_type& front() const
    {
      correct_ = false;
      return container_type::front();
    }

    iterator begin()
    {
      correct_ = false;
      return container_type::begin();
    }

    iterator end()
    {
      correct_ = false;
      return container_type::end();
    }

    template<typename IN>
    void insert(const IN& _t)
    {
      correct_ = false;
      container_type::insert(_t);
    }

    template<typename INPUT_ITERATOR, typename OUTPUT_ITERATOR>
    void insert(INPUT_ITERATOR _a, OUTPUT_ITERATOR _b, OUTPUT_ITERATOR _c)
    {
      correct_ = false;
      container_type::insert(_a,_b,_c);
    }

    void push_back(const value_type& _v)
    {
      correct_ = false;
      container_type::push_back(_v);
    }

    void push_back(value_type&& _v)
    {
      correct_ = false;
      container_type::push_back(_v);
    }

    void update()
    {
      correct_ = true;
    }

    bool correct() const { return correct_; }

  protected:
    bool correct_;
  };

}

#pragma once
#include "Base.h"
#include "Traits.h"
#include "Verify.h"

namespace Ty {

template <typename T>
struct View {
    constexpr View(T* data, usize size)
        : m_data(data)
        , m_size(size)
    {
    }

    T& operator[](usize index)
    {
        VERIFY(index < size());
        VERIFY(m_data);
        return m_data[index];
    }

    T const& operator[](usize index) const 
    {
        VERIFY(index < size());
        VERIFY(m_data);
        return m_data[index];
    }

    constexpr T* begin()
    {
        VERIFY(m_data);
        return m_data;
    }
    constexpr T* end()
    {
        VERIFY(m_data);
        return &m_data[m_size];
    }

    constexpr T const* begin() const
    {
        VERIFY(m_data);
        return m_data;
    }
    constexpr T const* end() const
    { 
        VERIFY(m_data);
        return &m_data[m_size];
    }

    constexpr usize size() const { return m_size; }
    constexpr T* data()
    {
        VERIFY(m_data);
        return m_data;
    }
    constexpr T const* data() const
    {
        VERIFY(m_data);
        return m_data;
    }

private:
    T* m_data;
    usize m_size;
};

}

using namespace Ty;

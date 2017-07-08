/**
 * @file
 * @date 2017/4/30
 */
#pragma once
#include <cassert>
#include <cstdint>

namespace moe
{
    /**
     * @brief 数组视图
     * @tparam T 元素类型
     *
     * 用来指示一段具有T类型的数组，不托管所有权。
     */
    template <typename T>
    class ArrayView
    {
    public:
        ArrayView()noexcept
            : m_pBuffer(nullptr), m_uSize(0) {}

        ArrayView(const T* data, size_t size)noexcept
            : m_pBuffer(data), m_uSize(size)
        {
            assert(size == 0 || (size > 0 && data != nullptr));
        }

        explicit operator bool()const noexcept { return m_pBuffer != nullptr; }

        const T& operator[](size_t index)const noexcept
        {
            assert(0 <= index && index < m_uSize);
            return m_pBuffer[index];
        }

    public:
        /**
         * @brief 获取缓冲区大小
         * @return 缓冲区元素个数
         */
        size_t GetSize()const noexcept { return m_uSize; }

        /**
         * @brief 缓冲区是否为空
         * @return 是否空
         */
        bool IsEmpty()const noexcept { return m_uSize == 0; }

        /**
         * @brief 获取缓冲区
         * @return 缓冲区指针
         */
        const T* GetBuffer()const noexcept { return m_pBuffer; }

        /**
         * @brief 获取第一个元素
         * @return 第一个元素
         */
        const T& First()const noexcept
        {
            assert(!IsEmpty());
            return m_pBuffer[0];
        }

        /**
         * @brief 获取最后一个元素
         * @return 最后一个元素
         */
        const T& Last()const noexcept
        {
            assert(!IsEmpty());
            return m_pBuffer[m_uSize - 1];
        }

        /**
         * @brief 原地截取一段缓冲区
         * @param from 起始位置
         * @param to 结束位置
         * @return 具有相同内存位置的片段
         */
        ArrayView<T> Slice(size_t from, size_t to)const noexcept
        {
            assert(from <= to);
            assert(to <= m_uSize);
            return ArrayView<T>(GetBuffer() + from, to - from);
        }

    protected:
        const T* m_pBuffer;
        size_t m_uSize;
    };

    /**
     * @brief 可变数组视图
     * @tparam T 元素类型
     *
     * 用来指示一段具有T类型的数组，不托管所有权。
     */
    template <typename T>
    class MutableArrayView :
        public ArrayView<T>
    {
    public:
        using ArrayView<T>::ArrayView;
        using ArrayView<T>::operator[];

        T& operator[](size_t index)noexcept
        {
            assert(0 <= index && index < ArrayView<T>::m_uSize);
            return const_cast<T*>(ArrayView<T>::m_pBuffer)[index];
        }

    public:
        using ArrayView<T>::GetBuffer;
        using ArrayView<T>::First;
        using ArrayView<T>::Last;
        using ArrayView<T>::Slice;

        /**
         * @brief 获取缓冲区
         * @return 缓冲区指针
         */
        T* GetBuffer()noexcept { return const_cast<T*>(ArrayView<T>::m_pBuffer); }

        /**
         * @brief 获取第一个元素
         * @return 第一个元素
         */
        T& First()noexcept
        {
            assert(!ArrayView<T>::IsEmpty());
            return const_cast<T*>(ArrayView<T>::m_pBuffer)[0];
        }

        /**
         * @brief 获取最后一个元素
         * @return 最后一个元素
         */
        T& Last()noexcept
        {
            assert(!ArrayView<T>::IsEmpty());
            return const_cast<T*>(ArrayView<T>::m_pBuffer)[ArrayView<T>::m_uSize - 1];
        }

        /**
         * @brief 原地截取一段缓冲区
         * @param from 起始位置
         * @param to 结束位置
         * @return 具有相同内存位置的片段
         */
        MutableArrayView<T> Slice(size_t from, size_t to)noexcept
        {
            assert(to <= ArrayView<T>::m_uSize);
            assert(from < to);
            assert(0 <= from);
            return MutableArrayView<T>(GetBuffer() + from, to - from);
        }
    };

    using BytesView = ArrayView<uint8_t>;
    using MutableBytesView = MutableArrayView<uint8_t>;
}

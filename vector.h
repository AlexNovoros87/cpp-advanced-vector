#pragma once
// VECTOR.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <cassert>
#include <memory>
#include <type_traits>

/*
УВАЖАЕМЫЙ(АЯ) РЕВЬЮЕР, В КЛАССАХ МЕТОДЫ УМЕЩАЮЩИЕСЯ В ОДНУ СТРОЧКУ ТИПА RETURN ...
ИЛИ ВЫПОЛНЯЮЩИЕ ОДНУ ФУНКЦИЮ СЧИТАЮ НЕРАЦИОЛНАЛЬНЫМ ВЫНОСИТЬ ЗА КЛАСС
*/


template <typename T>
class RawMemory {
public:
    
    //КОНСТРУКТ - ДЕСТРУКТ
    RawMemory() = default;
    RawMemory(const RawMemory&) = delete;
    explicit RawMemory(size_t capacity);
    RawMemory(RawMemory&& other) noexcept;
    ~RawMemory() { Deallocate(buffer_); }
   
    
    //МЕТОДЫ
    T* GetAddress() noexcept { return buffer_; }
    size_t Capacity() const { return capacity_; }
    const T* GetAddress() const noexcept { return buffer_; }
    void Swap(RawMemory& other) noexcept;
    
    
    //ОПЕРАТОРЫ
    RawMemory& operator=(const RawMemory& rhs) = delete;
    RawMemory& operator=(RawMemory&& rhs) noexcept;
    T* operator+(size_t offset) noexcept;
    const T* operator+(size_t offset) const noexcept;
    const T& operator[](size_t index) const noexcept;
    T& operator[](size_t index) noexcept;

private:
    // Выделяет сырую память под n элементов и возвращает указатель на неё
    static T* Allocate(size_t n);
    // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
    static void Deallocate(T* buf) noexcept;
    T* buffer_ = nullptr;
    size_t capacity_ = 0;
};


template <typename T>
class Vector {
public:

    //ПСЕВДОНИМЫ И ИТЕРАТОРЫ
    using iterator = T*;
    using const_iterator = const T*;
    iterator begin() noexcept { return data_.GetAddress(); }
    iterator end() noexcept { return data_.GetAddress() + size_; }
    const_iterator begin() const noexcept { return data_.GetAddress(); };
    const_iterator end() const noexcept { return data_.GetAddress() + size_; }
    const_iterator cbegin() const noexcept { return data_.GetAddress(); }
    const_iterator cend() const noexcept { return data_.GetAddress() + size_; }

    //КОНСТРУКТ - ДЕСТРУКТ
    Vector();
    Vector(const Vector& other);
    Vector(size_t sz);
    Vector(Vector&& rhs) noexcept;
    ~Vector();

    //СЛУЖЕБНЫЕ ПУБЛИЧНЫЕ
    void Reserve(size_t new_capacity);
    size_t Size() const noexcept;
    size_t Capacity() const noexcept;
    Vector<T>& Swap(Vector& other) noexcept;

    iterator Erase(const T* it) noexcept;
    iterator Insert(const T* pos, const T& value);
    iterator Insert(const T* pos, T&& value);

    template <typename... Args>
    iterator Emplace(const T* it, Args&&... val);

    template <typename... Args>
    T& EmplaceBack(Args&&... args);
    Vector<T>& Resize(size_t sz);

    template <typename Type>
    void PushBack(Type&& value);
    Vector<T>& PopBack();


    //ОПЕРАТОРЫ
    const T& operator[](size_t index) const noexcept;
    T& operator[](size_t index) noexcept;
    Vector& operator=(const Vector& rhs);
    Vector& operator=(Vector&& rhs) noexcept;


private:
    void FromThisDataToNewMemory(RawMemory<T>& memory, size_t startpos, size_t endpos, size_t mempos);
    RawMemory<T> data_;
    size_t size_ = 0;
};
///////////////////////////////////////////////////////////////
////
///        RAW_MEMORY   RAW_MEMORY   RAW_MEMORY 
///
//////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
//
//                  КОНСТРУКТОРЫ
//
/////////////////////////////////////////////////////////

template <typename T>
RawMemory<T>::RawMemory(size_t capacity)
    : buffer_(Allocate(capacity))
    , capacity_(capacity) {
}


template <typename T>
RawMemory<T>::RawMemory(RawMemory&& other) noexcept {
    buffer_ = other.buffer_;
    capacity_ = other.capacity_;
    other.buffer_ = nullptr;
    other.capacity_ = 0;
}

/////////////////////////////////////////////////////////
//
//                  ОПЕРАТОРЫ
//
/////////////////////////////////////////////////////////

template <typename T>
RawMemory<T>& RawMemory<T>::operator=(RawMemory&& rhs) noexcept {

    if (this != &rhs) {
        Deallocate(buffer_);
        buffer_ = rhs.buffer_;
        capacity_ = rhs.capacity_;
        rhs.buffer_ = nullptr;
        rhs.capacity_ = 0;
    }
    return *this;
}

template <typename T>
T* RawMemory<T>::operator+(size_t offset) noexcept {
    // Разрешается получать адрес ячейки памяти, следующей за последним элементом массива
    assert(offset <= capacity_);
    if (buffer_ != nullptr && offset <= capacity_) return buffer_ + offset;
    return nullptr;
}

template <typename T>
const T* RawMemory<T>::operator+(size_t offset) const noexcept {
    return const_cast<RawMemory&>(*this) + offset;
}

template <typename T>
const T& RawMemory<T>::operator[](size_t index) const noexcept {
    assert(index < capacity_);
    return const_cast<RawMemory&>(*this)[index];
}

template <typename T>
T& RawMemory<T>::operator[](size_t index) noexcept {
    assert(index < capacity_);
    return buffer_[index];
}

/////////////////////////////////////////////////////////
//
//                 МЕТОДЫ
//
/////////////////////////////////////////////////////////

// Выделяет сырую память под n элементов и возвращает указатель на неё
template <typename T>
T* RawMemory<T>::Allocate(size_t n) {
    return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
}

// Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
template <typename T>
void RawMemory<T>::Deallocate(T* buf) noexcept {
    operator delete(buf);
    buf = nullptr;
}

template <typename T>
void RawMemory<T>::Swap(RawMemory& other) noexcept {
    std::swap(buffer_, other.buffer_);
    std::swap(capacity_, other.capacity_);
}

///////////////////////////////////////////////////////////////
////
///                 VECTOR VECTOR VECTOR
///
//////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////
//
//             КОНСТРУКТОРЫ - ДЕСТРУКТОР
//
/////////////////////////////////////////////////////////

template <typename T>
Vector<T>::Vector() = default;

template <typename T>
Vector<T>::Vector(const Vector& other)
    : data_(other.size_)
    , size_(other.size_)
{
    std::uninitialized_copy(other.data_.GetAddress(), other.data_.GetAddress() + other.size_, data_.GetAddress());
}

template <typename T>
Vector<T>::Vector(size_t sz)
    : data_(sz)
    , size_(sz)
{
    std::uninitialized_value_construct_n(data_.GetAddress(), sz);
}

template <typename T>
Vector<T>::Vector(Vector&& rhs) noexcept {
    std::swap(size_, rhs.size_);
    data_.Swap(rhs.data_);
}

template <typename T>
Vector<T>::~Vector() {
    std::destroy_n(data_.GetAddress(), size_);
}

/////////////////////////////////////////////////////////
//
//               СЛУЖЕБНЫЕ ПУБЛИЧНЫЕ
//
/////////////////////////////////////////////////////////

template <typename T>
typename Vector<T>::iterator Vector<T>::Insert(const T* pos, const T& value) {
    return Emplace(pos, value);
}

template <typename T>
typename Vector<T>::iterator Vector<T>::Insert(const T* pos, T&& value) {
    return Emplace(pos, std::move(value));
}

template <typename T>
typename Vector<T>::iterator Vector<T>::Erase(const T* it) noexcept {
    size_t pos = std::distance(cbegin(), it);
    std::move(begin() + pos + 1, end(), begin() + pos);
    PopBack();
    return begin() + pos;
}


template <typename T>
template <typename... Args>
typename Vector<T>::iterator Vector<T>::Emplace(const T* it, Args&&... val) {

    size_t pos = std::distance(cbegin(), it);
    if (it == cend() || size_ == 0) {
        EmplaceBack(std::forward<Args>(val)...);
        return begin() + pos;
    };

    iterator answer = nullptr;

    if (size_ == Capacity()) {

        size_t new_locator_size = size_ == 0 ? 1 : size_ * 2;
        RawMemory<T> new_data(new_locator_size);
        answer = new(new_data.GetAddress() + pos) T(std::forward<Args>(val)...);

        try {
            FromThisDataToNewMemory(new_data, 0, pos, 0);
            FromThisDataToNewMemory(new_data, pos, size_, pos + 1);
        }
        catch (...) {
            std::destroy_n(new_data.GetAddress() + pos, 1);
            throw;
        }
        std::destroy_n(data_.GetAddress(), size_);
        data_.Swap(new_data);
    }
    else {

        T tmp(std::forward<Args>(val)...);
        new (data_ + size_) T(std::move(data_[size_ - 1]));
        try {
            std::move_backward(data_ + pos, end() - 1, end());
        }
        catch (...) {
            std::destroy_at(data_ + size_);
        }
        data_[pos] = std::move(tmp);
        answer = begin() + pos;
    }
    ++size_;
    return answer;
}

template <typename T>
template <typename... Args>
T& Vector<T>::EmplaceBack(Args&&... args) {
    if (size_ == Capacity()) {
        RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
        new (new_data + size_) T(std::forward<Args>(args)...);
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
            std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
        }
        else {
            std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
        }
        std::destroy_n(data_.GetAddress(), size_);
        data_.Swap(new_data);
    }
    else {
        new (data_ + size_) T(std::forward<Args>(args)...);
    }
    ++size_;
    return data_[size_ - 1];
}

template <typename T>
Vector<T>& Vector<T>::Resize(size_t sz) {
    if (sz == size_) return *this;

    if (sz < size_) {
        std::destroy_n(data_.GetAddress() + sz, size_ - sz);
        size_ = sz;
    }
    else {
        Reserve(sz);
        std::uninitialized_value_construct_n(data_.GetAddress() + size_, sz - size_);
        size_ = sz;
    }
    return *this;
}

template <typename T>
template <typename Type>
void Vector<T>::PushBack(Type&& value) {
    if (size_ == Capacity()) {
        RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
        new (new_data + size_) T(std::forward<Type>(value));
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
            std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
        }
        else {
            std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
        }
        std::destroy_n(data_.GetAddress(), size_);
        data_.Swap(new_data);
    }
    else {
        new (data_ + size_) T(std::forward<Type>(value));
    }
    ++size_;
}

template <typename T>
Vector<T>& Vector<T>::PopBack() {
    std::destroy_at(data_.GetAddress() + (size_ - 1));
    --size_;
    return *this;
}


template <typename T>
Vector<T>& Vector<T>::Swap(Vector& other) noexcept {
    data_.Swap(other.data_);
    std::swap(size_, other.size_);
    return *this;
}

template <typename T>
void Vector<T>::Reserve(size_t new_capacity) {
    if (new_capacity <= data_.Capacity()) {
        return;
    }
    RawMemory<T> new_data(new_capacity);

    /*
    Перемещайте элементы, только если соблюдается хотя бы одно из условий:
    конструктор перемещения типа T не выбрасывает исключений;
    тип T не имеет копирующего конструктора.
    В противном случае элементы надо копировать.
    */

    if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
        std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
    }
    else {
        std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
    }
    std::destroy_n(data_.GetAddress(), size_);
    data_.Swap(new_data);

}

template <typename T>
size_t Vector<T>::Size() const noexcept {
    return size_;
}

template <typename T>
size_t Vector<T>::Capacity() const noexcept {
    return data_.Capacity();
}

/////////////////////////////////////////////////////////
//
//               ОПЕРАТОРЫ
//
/////////////////////////////////////////////////////////

template <typename T>
const T& Vector<T>::operator[](size_t index) const noexcept {
    return const_cast<Vector&>(*this)[index];
}

template <typename T>
T& Vector<T>::operator[](size_t index) noexcept {
    assert(index < size_);
    return data_[index];
}

template <typename T>
Vector<T>& Vector<T>::operator=(const Vector& rhs) {
  
    if (this != &rhs) {
        if (rhs.size_ > data_.Capacity()) {
            Vector rhs_copy(rhs);
            Swap(rhs_copy);
        }
        else {
            if (rhs.size_ < size_) {
                std::copy(rhs.data_.GetAddress(), rhs.data_.GetAddress() + rhs.size_, data_.GetAddress());
                std::destroy_n(data_.GetAddress() + rhs.size_, size_ - rhs.size_);
            }
            else {
                std::copy(rhs.data_.GetAddress(), rhs.data_.GetAddress() + size_, data_.GetAddress());
                std::uninitialized_copy_n(rhs.data_.GetAddress() + size_, rhs.size_ - size_, data_.GetAddress() + size_);
            }
            size_ = rhs.size_;
        }
    }
    return *this;
}

template <typename T>
Vector<T>& Vector<T>::operator=(Vector&& rhs) noexcept {
    std::swap(size_, rhs.size_);
    data_.Swap(rhs.data_);
    return *this;
}

template <typename T>
void Vector<T>::FromThisDataToNewMemory(RawMemory<T>& memory, size_t startpos, size_t endpos, size_t mempos) {
    if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
        std::uninitialized_move_n(data_.GetAddress() + startpos, endpos - startpos, memory.GetAddress() + mempos);
    }
    else {
        std::uninitialized_copy_n(data_.GetAddress() + startpos, endpos - startpos, memory.GetAddress() + mempos);
    }
}
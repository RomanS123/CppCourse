#include <math.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <type_traits>
#include <vector>

template <typename T, typename U>
int DivAndCeil(T numerator, U denominator) {
  return std::ceil((double)numerator / denominator);
}

const size_t kEight = 8;
template <typename T, size_t S = kEight>
class Deque {
 private:
  size_t size_ = 0;
  size_t n_arrays_ = 0;  // real number of arrays

  T** data_ = nullptr;
  T* end_ = nullptr;
  T* start_ = nullptr;
  size_t first_arr_ = 0;
  size_t main_capacity_ = 0;  // capacity of main array
 public:
  template <bool IsConst>
  class CommonIterator {
    friend class Deque;

   private:
    T** ptr_;     //указатель в внешнем массиве
    int number_;  //номер во внутреннем массиве
   public:
    using iterator_category =
        std::random_access_iterator_tag;  // type of our iterator
    using difference_type = std::ptrdiff_t;
    using value_type = std::conditional_t<IsConst, const T, T>;
    using pointer =
        std::conditional_t<IsConst, const T*, T*>;  // or value_type*
    using double_pointer = std::conditional_t<IsConst, const T**, T**>;
    using reference =
        std::conditional_t<IsConst, const T&, T&>;  // or value_type&
    CommonIterator() = default;
    CommonIterator(T** ptr_external, T* ptr_internal) {
      ptr_ = ptr_external;
      if (ptr_internal == nullptr || ptr_ == nullptr) {
        number_ = 0;
        ptr_ = nullptr;
      } else {
        number_ = ptr_internal - *ptr_;
      }
    }
    CommonIterator& operator++() {
      *this += 1;
      return *this;
    }
    CommonIterator operator++(int) {
      CommonIterator old = *this;
      this->operator++();
      return old;
    }
    CommonIterator& operator--() {
      *this -= 1;
      return *this;
    }
    friend CommonIterator operator+(CommonIterator iter, difference_type num) {
      iter += num;
      return iter;
    }
    friend CommonIterator operator+(difference_type num, CommonIterator iter) {
      iter += num;
      return iter;
    }
    CommonIterator& operator-=(difference_type shift) {
      int tmp = std::ceil(std::abs(shift - number_) / (double)S);
      if (shift > number_) {
        ptr_ -= tmp;
      }
      number_ = (number_ - shift + S) % S;
      return *this;
    }
    CommonIterator& operator+=(difference_type shift) {
      ptr_ += (number_ + shift) / S;
      number_ = (number_ + shift) % S;
      return *this;
    }
    reference operator*() const { return *(*ptr_ + number_); }
    pointer operator->() const { return *ptr_ + number_; }
    friend CommonIterator operator-(CommonIterator iter, difference_type num) {
      iter -= num;
      return iter;
    }
    template <bool Const>
    difference_type operator-(const CommonIterator<Const>& other) const {
      if (ptr_ == nullptr) {
        return 0;
      }
      return (ptr_ - other.ptr_) * S + (number_ - other.number_);
    }
    template <bool Const>
    bool operator<(const CommonIterator<Const>& other) const {
      if (ptr_ == other.ptr_) {
        return number_ < other.number_;
      }
      return ptr_ < other.ptr_;
    }
    template <bool Const>
    bool operator>(const CommonIterator<Const>& other) const {
      return !(*this <= other);
    }
    template <bool Const>
    bool operator==(const CommonIterator<Const>& other) const {
      return ((ptr_ == other.ptr_) && (number_ == other.number_));
    }
    template <bool Const>
    bool operator!=(const CommonIterator<Const>& other) const {
      return !(*this == other);
    }
    template <bool Const>
    bool operator<=(const CommonIterator<Const>& other) const {
      return (*this < other) || *this == other;
    }
    template <bool Const>
    bool operator>=(const CommonIterator<Const>& other) const {
      return (*this < other) || *this == other;
    }
  };
  using iterator = CommonIterator<false>;
  using const_iterator = CommonIterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() { return iterator(data_ + first_arr_, start_); }
  iterator end() {
    auto tmp = data_ + first_arr_ + (n_arrays_ - 1);
    return iterator(tmp, end_) + !empty();
  }
  iterator begin() const { return iterator(data_ + first_arr_, start_); }
  iterator end() const {
    auto tmp = data_ + first_arr_ + (n_arrays_ - 1);
    return iterator(tmp, end_) + !empty();
  }
  const_iterator cbegin() const {
    return const_iterator(data_ + first_arr_, start_);
  }
  const_iterator cend() const {
    auto tmp = data_ + first_arr_ + (n_arrays_ - 1);
    return const_iterator(tmp, end_) + !empty();
  }
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
  reverse_iterator rend() const { return const_reverse_iterator(begin()); }

  Deque(const Deque& other)
      : n_arrays_(other.n_arrays_), first_arr_(0), main_capacity_(n_arrays_) {
    auto other_size = other.size();
    auto iter = other.begin();
    auto begin = other.begin();
    auto end = other.end() - 1;

    size_t needed = DivAndCeil(other_size, S);
    if (!other.empty()) {
      data_ = new T*[needed];
      for (size_t i = 0; i < needed; ++i) {
        data_[i] = reinterpret_cast<T*>(new int8_t[S * sizeof(T)]);
      }
      try {
        while (iter != other.end()) {
          new (data_[iter.ptr_ - begin.ptr_] + iter.number_) T(*iter);
          ++iter;
          ++size_;
        }
        start_ = data_[0] + begin.number_;
        end_ = data_[end.ptr_ - begin.ptr_] + end.number_;
      } catch (...) {
        while (iter != other.begin()) {
          auto external_num = iter.ptr_ - other.data_;
          (data_[external_num] + iter.number_)->~T();
          --iter;
        }
        for (size_t i = 0; i < n_arrays_; ++i) {
          delete[] reinterpret_cast<int8_t*>(data_[i]);
        }
        delete[] data_;
        throw;
      }
    }
  }

  void swap(Deque& other) {
    std::swap(size_, other.size_);
    std::swap(n_arrays_, other.n_arrays_);
    std::swap(start_, other.start_);
    std::swap(end_, other.end_);
    std::swap(data_, other.data_);
    std::swap(first_arr_, other.first_arr_);
    std::swap(main_capacity_, other.main_capacity_);
  }
  Deque& operator=(const Deque<T>& other) {
    Deque<T> copy(other);
    this->swap(copy);
    return *this;
  }
  Deque() = default;

  void constr_helper(T* ptr, const T& value) { new (ptr) T(value); }
  void constr(size_t count, const T& value) {
    int needed = DivAndCeil(count, S);
    if (count > 0) {
      count = count % S;
      try {
        data_ = new T*[needed];
        for (int i = 0; i < needed - 1; ++i) {
          data_[i] = reinterpret_cast<T*>(new int8_t[S * sizeof(T)]);
          ++n_arrays_;
          for (size_t j = 0; j < S; ++j) {
            ++size_;
            constr_helper(data_[i] + j, value);
          }
        }
        ++n_arrays_;
        data_[needed - 1] = reinterpret_cast<T*>(new int8_t[S * sizeof(T)]);
        if (needed > 0 && count == 0) {
          count = S;
        }
        for (size_t j = 0; j < count; ++j) {
          ++size_;
          constr_helper(data_[needed - 1] + j, value);
        }
        main_capacity_ = n_arrays_;
        start_ = data_[0];
        end_ = data_[needed - 1] + (count - 1);
        if (needed > 0 && count == 0) {
          end_ = data_[needed - 1] + (S - 1);
        }
      } catch (...) {
        handle_constr();
        throw;
      }
    }
  }
  void constr(size_t count) {
    if constexpr (std::is_default_constructible<T>::value) {
      constr(count, T());
    }
  }
  Deque(size_t count) { constr(count); }
  Deque(size_t count, const T& value) { constr(count, value); }
  void handle_constr() {
    for (size_t i = 0; i < size_; ++i) {
      (data_[i / S] + (i % S))->~T();
    }
    for (size_t i = 0; i < n_arrays_; ++i) {
      delete[] reinterpret_cast<int8_t*>(data_[i]);
    }
    delete[] data_;
    size_ = 0;
  }
  template <bool NoThrow = false>
  T& at(size_t num) {
    if (num > size() - 1 && !NoThrow) {
      throw std::out_of_range("index error");
    }
    auto iter = begin();
    iter += num;
    return *iter;
  }
  void print() {
    auto iter = begin();
    while (iter != end()) {
      std::cout << *iter;
      ++iter;
    }
  }
  template <bool NoThrow = false>
  const T& at(size_t num) const {
    if (num > size() - 1 && !NoThrow) {
      throw std::out_of_range("index error");
    }
    auto iter = begin();
    iter += num;
    return *iter;
  }
  T& operator[](size_t num) { return at<true>(num); }
  const T& operator[](size_t num) const { return at<true>(num); }
  size_t size() const { return size_; };
  bool empty() const { return (size_ == 0); }
  void handle_push(T** arr, size_t ind) {
    delete[] reinterpret_cast<int8_t*>(arr[ind]);
    delete[] arr;
    ++first_arr_;
    --n_arrays_;
  }
  void push_back(const T& elem) {
    if (empty()) {
      *this = Deque<T>(1, elem);
      return;
    }
    if (end_ + 1 > data_[first_arr_ + (n_arrays_ - 1)] + (S - 1)) {
      if (data_[first_arr_ + (n_arrays_ - 1)] == data_[main_capacity_ - 1]) {
        auto needed = DivAndCeil(n_arrays_, S);

        auto new_arr = new T*[main_capacity_ + needed * 2];

        auto num = DivAndCeil((main_capacity_ + needed * 2 - n_arrays_), 2);
        for (size_t i = 0; i < n_arrays_; ++i) {
          new_arr[num + i] = *(data_ + first_arr_ + i);
        }
        first_arr_ = num;
        new_arr[first_arr_ + n_arrays_] =
            reinterpret_cast<T*>(new int8_t[S * sizeof(T)]);
        end_ = new_arr[first_arr_ + n_arrays_];
        try {
          new (end_) T(elem);
          ++n_arrays_;
        } catch (...) {
          handle_push(new_arr, first_arr_ + n_arrays_);
          throw;
        }
        delete[] data_;
        data_ = new_arr;
        main_capacity_ = main_capacity_ + needed * 2;
      } else {
        // internal
        ++n_arrays_;
        data_[first_arr_ + (n_arrays_ - 1)] =
            reinterpret_cast<T*>(new int8_t[S * sizeof(T)]);
        end_ = data_[first_arr_ + (n_arrays_ - 1)];
        try {
          new (end_) T(elem);
        } catch (...) {
          delete[] reinterpret_cast<int8_t*>(
              data_[first_arr_ + (n_arrays_ - 1)]);
          --n_arrays_;
          end_ = data_[first_arr_ + (n_arrays_ - 1)] + (S - 1);
          throw;
        }
      }
    } else {
      // just push
      try {
        new (end_ + 1) T(elem);
        ++end_;
      } catch (...) {
        throw;
      }
    }
    ++size_;
  }
  void push_front(const T& elem) {
    if (size() == 0) {
      *this = Deque(1, elem);
      return;
    }
    if (start_ - 1 < data_[first_arr_]) {
      if ((first_arr_ == 0 && start_ == data_[first_arr_])) {
        size_t needed = DivAndCeil(n_arrays_, 2);
        T** new_arr = new T*[main_capacity_ + needed * 2];
        for (size_t i = 0; i < n_arrays_; ++i) {
          new_arr[needed + i] = *(data_ + first_arr_ + i);
        }
        first_arr_ = needed - 1;

        ++n_arrays_;
        new_arr[first_arr_] = reinterpret_cast<T*>(new int8_t[S * sizeof(T)]);
        try {
          start_ = new_arr[first_arr_] + (S - 1);
          new (start_) T(elem);
        } catch (...) {
          handle_push(new_arr, first_arr_);
          start_ = data_[first_arr_];
          throw;
        }
        delete[] data_;
        data_ = new_arr;
        main_capacity_ = main_capacity_ + needed * 2;
      } else {
        --first_arr_;
        ++n_arrays_;
        data_[first_arr_] = reinterpret_cast<T*>(new int8_t[S * sizeof(T)]);
        start_ = data_[first_arr_] + (S - 1);
        try {
          new (start_) T(elem);
        } catch (...) {
          delete[] reinterpret_cast<int8_t*>(data_[first_arr_]);
          ++first_arr_;
          --n_arrays_;
          start_ = data_[first_arr_];
          throw;
        }
      }
    } else {
      try {
        start_ -= 1;
        new (start_) T(elem);
      } catch (...) {
        throw;
      }
    }
    ++size_;
  }
  void pop_front() {
    start_->~T();
    if (start_ + 1 > data_[first_arr_] + (S - 1)) {
      // delete array
      delete[] reinterpret_cast<int8_t*>(data_[first_arr_]);
      ++first_arr_;
      --n_arrays_;
      if (n_arrays_ > 0) {
        start_ = data_[first_arr_];
      }
    } else {
      ++start_;
    }
    --size_;
  }
  void pop_back() {
    end_->~T();
    --size_;
    if (end_ - 1 < data_[first_arr_ + (n_arrays_ - 1)]) {
      // delete array
      delete[] reinterpret_cast<int8_t*>(data_[first_arr_ + (n_arrays_ - 1)]);
      --n_arrays_;
      end_ = data_[first_arr_ + (n_arrays_ - 1)] + (S - 1);
    } else {
      --end_;
    }
  }
  void insert(iterator iter, const T& elem) {
    auto it1 = begin();
    auto it2 = end();
    if (iter <= it2) {
      auto new_deque = Deque<T>(0);
      while (it1 != iter) {
        new_deque.push_back(*it1);
        ++it1;
      }
      new_deque.push_back(elem);
      while (it1 != it2) {
        new_deque.push_back(*it1);
        ++it1;
      }
      *this = new_deque;
    }
  }
  void erase(iterator iter) {
    auto it1 = begin();
    auto it2 = end();

    auto new_deque = Deque<T>(0);
    while (it1 != iter) {
      new_deque.push_back(*it1);
      ++it1;
    }
    ++it1;
    while (it1 != it2) {
      new_deque.push_back(*it1);
      ++it1;
    }

    *this = new_deque;
  }
  ~Deque() {
    if (!empty()) {
      auto iter = begin();
      while (iter != end()) {
        auto external_num = iter.ptr_ - data_;
        (data_[external_num] + iter.number_)->~T();
        ++iter;
      }
      for (size_t i = 0; i < n_arrays_; ++i) {
        delete[] reinterpret_cast<int8_t*>(data_[first_arr_ + i]);
      }
    }
    delete[] data_;
  }
};

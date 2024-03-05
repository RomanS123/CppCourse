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
template <typename T, typename Allocator = std::allocator<T>, size_t S = kEight>
class Deque {
 private:
  size_t size_ = 0;
  size_t n_arrays_ = 0;  // real number of arrays

  using external =
      typename std::allocator_traits<Allocator>::template rebind_alloc<T*>;
  using external_alloc_traits = std::allocator_traits<external>;
  using alloc_traits = std::allocator_traits<Allocator>;

  Allocator alloc_;
  external external_alloc_;

  T** data_ = nullptr;
  T* start_ = nullptr;
  T* end_ = nullptr;
  size_t first_arr_ = 0;
  size_t main_capacity_ = 0;  // capacity of main array
 public:
  using value_type = T;
  using allocator_type = Allocator;
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
      : size_(other.size_),
        n_arrays_(DivAndCeil(size_, S)),
        alloc_(std::allocator_traits<
               Allocator>::select_on_container_copy_construction(other.alloc_)),
        external_alloc_(alloc_),
        data_(external_alloc_traits::allocate(external_alloc_, n_arrays_)),
        first_arr_(0),
        main_capacity_(n_arrays_) {
    if (!other.empty()) {
      for (size_t i = 0; i < n_arrays_; ++i) {
        data_[i] = alloc_traits::allocate(alloc_, S);
      }
      start_ = data_[0];
      end_ = data_[n_arrays_ - 1] + (size_ - (n_arrays_ - 1) * S - 1);
      auto iter = begin();
      auto other_iter = other.begin();
      try {
        while (other_iter != other.end()) {
          alloc_traits::construct(alloc_, &(*iter), *other_iter);
          ++other_iter;
          ++iter;
        }
      } catch (...) {
        while (iter != begin()) {
          --iter;
          alloc_traits::destroy(alloc_, &(*iter));
        }
        for (size_t i = 0; i < n_arrays_; ++i) {
          alloc_traits::deallocate(alloc_, data_[i], S);
        }
        external_alloc_traits::deallocate(external_alloc_, data_, n_arrays_);
        throw;
      }
    }
  }
  Deque(Deque&& other)
      : size_(other.size_),
        n_arrays_(other.n_arrays_),
        alloc_(
            std::move(std::allocator_traits<Allocator>::
                          select_on_container_copy_construction(other.alloc_))),
        external_alloc_(alloc_),
        data_(other.data_),
        start_(other.start_),
        end_(other.end_),
        first_arr_(other.first_arr_),
        main_capacity_(other.first_arr_) {
    other.size_ = 0;
    other.n_arrays_ = 0;
    other.data_ = nullptr;
    other.start_ = nullptr;
    other.end_ = nullptr;
    other.first_arr_ = 0;
    other.main_capacity_ = 0;
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
  Deque& operator=(const Deque& other) {
    Deque<T, Allocator> copy(other);
    if (std::allocator_traits<
            Allocator>::propagate_on_container_copy_assignment::value &&
        alloc_ != other.alloc_ && !other.empty()) {
      alloc_ = other.alloc_;
      external_alloc_ = alloc_;
    }
    this->swap(copy);

    return *this;
  }
  Deque& operator=(Deque&& other) {
    size_ = other.size_;
    n_arrays_ = other.n_arrays_;
    if (std::allocator_traits<
            Allocator>::propagate_on_container_move_assignment::value &&
        alloc_ != other.alloc_ && !other.empty()) {
      alloc_ = std::move(other.alloc_);
    }
    external_alloc_ = alloc_;
    data_ = other.data_;
    start_ = other.start_;
    end_ = other.end_;
    first_arr_ = other.first_arr_;
    main_capacity_ = other.main_capacity_;

    other.size_ = 0;
    other.n_arrays_ = 0;
    other.start_ = other.end_;
    other.data_ = nullptr;
    other.start_ = nullptr;
    other.end_ = nullptr;
    return *this;
  }
  Deque() = default;
  Deque(size_t count, const Allocator& alloc = Allocator())
      : size_(count),
        n_arrays_(DivAndCeil(count, S)),
        alloc_(alloc),
        external_alloc_(alloc),
        data_(external_alloc_traits::allocate(external_alloc_, n_arrays_)),
        first_arr_(0),
        main_capacity_(n_arrays_) {
    if (empty()) {
      return;
    }
    for (size_t i = 0; i < n_arrays_; ++i) {
      data_[i] = alloc_traits::allocate(alloc_, S);
    }
    start_ = data_[0];
    end_ = data_[n_arrays_ - 1] + (size_ - (n_arrays_ - 1) * S - 1);
    auto iter = begin();
    if constexpr (std::is_default_constructible<T>::value) {
      while (iter != end()) {
        try {
          alloc_traits::construct(alloc_, &(*iter));
          ++iter;
        } catch (...) {
          while (iter != begin()) {
            --iter;
            alloc_traits::destroy(alloc_, &(*iter));
          }
          for (size_t i = 0; i < n_arrays_; ++i) {
            alloc_traits::deallocate(alloc_, data_[i], S);
          }
          external_alloc_traits::deallocate(external_alloc_, data_, n_arrays_);
          throw;
        }
      }
    }
  }
  Deque(size_t count, const T& value, const Allocator& alloc = Allocator())
      : size_(count),
        n_arrays_(DivAndCeil(count, S)),
        alloc_(alloc),
        external_alloc_(alloc),
        data_(external_alloc_traits::allocate(external_alloc_, n_arrays_)),
        first_arr_(0),
        main_capacity_(n_arrays_) {
    if (empty()) {
      return;
    }
    for (size_t i = 0; i < n_arrays_; ++i) {
      data_[i] = alloc_traits::allocate(alloc_, S);
    }
    start_ = data_[0];
    end_ = data_[n_arrays_ - 1] + (size_ - (n_arrays_ - 1) * S - 1);
    auto iter = begin();
    while (iter != end()) {
      try {
        alloc_traits::construct(alloc_, &(*iter), value);
        ++iter;
      } catch (...) {
        while (iter != begin()) {
          --iter;
          alloc_traits::destroy(alloc_, &(*iter));
        }
        for (size_t i = 0; i < n_arrays_; ++i) {
          alloc_traits::deallocate(alloc_, data_[i], S);
        }
        external_alloc_traits::deallocate(external_alloc_, data_,
                                          main_capacity_);
        throw;
      }
    }
  }

  Deque(std::initializer_list<T> init, const Allocator& alloc = Allocator())
      : size_(init.size()),
        n_arrays_(DivAndCeil(size_, S)),
        alloc_(alloc),
        external_alloc_(alloc),
        data_(external_alloc_traits::allocate(external_alloc_, size_)),
        first_arr_(0),
        main_capacity_(n_arrays_) {
    auto other_iter = init.begin();
    for (size_t i = 0; i < n_arrays_; ++i) {
      data_[i] = alloc_traits::allocate(alloc_, S);
    }
    start_ = data_[0];
    end_ = data_[n_arrays_ - 1] + (size_ - (n_arrays_ - 1) * S - 1);
    auto iter = begin();
    while (other_iter != init.end()) {
      try {
        alloc_traits::construct(alloc_, &(*iter), *other_iter);
      } catch (...) {
        while (iter != begin()) {
          alloc_traits::destroy(alloc_, &(*iter));
          --iter;
        }
        for (size_t i = 0; i < n_arrays_; ++i) {
          alloc_traits::deallocate(alloc_, data_[i], S);
        }
        external_alloc_traits::deallocate(external_alloc_, data_,
                                          main_capacity_);
        throw;
      }
      ++other_iter;
      ++iter;
    }
  }
  template <typename... Args>
  void additional(Args&&... args) {
    data_ = external_alloc_traits::allocate(external_alloc_, 1);
    data_[0] = alloc_traits::allocate(alloc_, S);
    try {
      alloc_traits::construct(alloc_, data_[0], std::forward<Args>(args)...);
    } catch (...) {
      alloc_traits::deallocate(alloc_, data_[0], S);
      external_alloc_traits::deallocate(external_alloc_, data_, 1);
      throw;
    }
    size_ = 1;
    n_arrays_ = 1;
    main_capacity_ = 1;
    start_ = data_[0];
    end_ = start_;
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
  Allocator get_allocator() { return alloc_; }

  void push_back(const T& elem) {
    if (empty()) {
      *this = Deque<T, Allocator>(1, elem);
      return;
    }
    if (end_ + 1 > data_[first_arr_ + (n_arrays_ - 1)] + (S - 1)) {
      if (data_[first_arr_ + (n_arrays_ - 1)] == data_[main_capacity_ - 1]) {
        auto needed = DivAndCeil(n_arrays_, S);
        auto fresh_arr = external_alloc_traits::allocate(
            external_alloc_, main_capacity_ + needed * 2);
        auto num = DivAndCeil((main_capacity_ + needed * 2 - n_arrays_), 2);

        for (size_t i = 0; i < n_arrays_; ++i) {
          fresh_arr[num + i] = *(data_ + first_arr_ + i);
        }
        first_arr_ = num;
        fresh_arr[first_arr_ + n_arrays_] = alloc_traits::allocate(alloc_, S);
        end_ = fresh_arr[first_arr_ + n_arrays_];
        try {
          alloc_traits::construct(alloc_, end_, elem);
          ++n_arrays_;
        } catch (...) {
          alloc_traits::deallocate(alloc_, fresh_arr[first_arr_ + n_arrays_],
                                   S);
          external_alloc_traits::deallocate(external_alloc_, fresh_arr,
                                            main_capacity_ + needed * 2);
          ++first_arr_;
          throw;
        }
        external_alloc_traits::deallocate(external_alloc_, data_,
                                          main_capacity_);
        data_ = fresh_arr;
        main_capacity_ = main_capacity_ + needed * 2;
      } else {
        // internal

        data_[first_arr_ + n_arrays_] = alloc_traits::allocate(alloc_, S);
        end_ = data_[first_arr_ + n_arrays_];
        try {
          alloc_traits::construct(alloc_, end_, elem);
          ++n_arrays_;
        } catch (...) {
          alloc_traits::deallocate(alloc_, data_[first_arr_ + n_arrays_], S);
          end_ = data_[first_arr_ + n_arrays_] + (S - 1);
          throw;
        }
      }
    } else {
      // just push
      try {
        alloc_traits::construct(alloc_, end_ + 1, elem);
        ++end_;
      } catch (...) {
        throw;
      }
    }
    ++size_;
  }
  void push_front(const T& elem) {
    if (empty()) {
      *this = Deque(1, elem);
      return;
    }
    if (start_ - 1 < data_[first_arr_]) {
      if ((first_arr_ == 0 && start_ == data_[first_arr_])) {
        size_t needed = DivAndCeil(n_arrays_, 2);
        T** fresh_arr = external_alloc_traits::allocate(
            external_alloc_, main_capacity_ + needed * 2);
        for (size_t i = 0; i < n_arrays_; ++i) {
          fresh_arr[needed + i] = *(data_ + first_arr_ + i);
        }
        first_arr_ = needed - 1;

        fresh_arr[first_arr_] = alloc_traits::allocate(alloc_, S);
        try {
          start_ = fresh_arr[first_arr_] + (S - 1);
          alloc_traits::construct(alloc_, start_, elem);
          ++n_arrays_;
        } catch (...) {
          alloc_traits::deallocate(alloc_, fresh_arr[first_arr_], S);
          external_alloc_traits::deallocate(external_alloc_, fresh_arr,
                                            main_capacity_ + needed * 2);
          ++first_arr_;
          start_ = data_[first_arr_];
          throw;
        }
        external_alloc_traits::deallocate(external_alloc_, data_,
                                          main_capacity_);
        data_ = fresh_arr;
        main_capacity_ = main_capacity_ + needed * 2;
      } else {
        data_[first_arr_ - 1] = alloc_traits::allocate(alloc_, S);
        start_ = data_[first_arr_ - 1] + (S - 1);
        try {
          alloc_traits::construct(alloc_, start_, elem);
          --first_arr_;
          ++n_arrays_;
        } catch (...) {
          alloc_traits::deallocate(alloc_, data_[first_arr_ - 1], S);
          start_ = data_[first_arr_];
          throw;
        }
      }
    } else {
      try {
        start_ -= 1;
        alloc_traits::construct(alloc_, start_, elem);
      } catch (...) {
        throw;
      }
    }
    ++size_;
  }
  template <typename... Args>
  void emplace_front(Args&&... args) {
    if (empty()) {
      additional(std::forward<Args>(args)...);
      return;
    }
    if (start_ - 1 < data_[first_arr_]) {
      if ((first_arr_ == 0 && start_ == data_[first_arr_])) {
        size_t needed = DivAndCeil(n_arrays_, 2);
        T** fresh_arr = external_alloc_traits::allocate(
            external_alloc_, main_capacity_ + needed * 2);
        for (size_t i = 0; i < n_arrays_; ++i) {
          fresh_arr[needed + i] = *(data_ + first_arr_ + i);
        }
        first_arr_ = needed - 1;

        fresh_arr[first_arr_] = alloc_traits::allocate(alloc_, S);
        try {
          start_ = fresh_arr[first_arr_] + (S - 1);
          alloc_traits::construct(alloc_, start_, std::forward<Args>(args)...);
          ++n_arrays_;
        } catch (...) {
          alloc_traits::deallocate(alloc_, fresh_arr[first_arr_], S);
          external_alloc_traits::deallocate(external_alloc_, fresh_arr,
                                            main_capacity_ + needed * 2);
          ++first_arr_;
          start_ = data_[first_arr_];
          throw;
        }
        external_alloc_traits::deallocate(external_alloc_, data_,
                                          main_capacity_);
        data_ = fresh_arr;
        main_capacity_ = main_capacity_ + needed * 2;
      } else {
        data_[first_arr_ - 1] = alloc_traits::allocate(alloc_, S);
        start_ = data_[first_arr_ - 1] + (S - 1);
        try {
          alloc_traits::construct(alloc_, start_, std::forward<Args>(args)...);
          --first_arr_;
          ++n_arrays_;
        } catch (...) {
          alloc_traits::deallocate(alloc_, data_[first_arr_ - 1], S);
          start_ = data_[first_arr_];
          throw;
        }
      }
    } else {
      try {
        start_ -= 1;
        alloc_traits::construct(alloc_, start_, std::forward<Args>(args)...);
      } catch (...) {
        throw;
      }
    }
    ++size_;
  }
  template <typename... Args>
  void emplace_back(Args&&... args) {
    if (empty()) {
      additional(std::forward<Args>(args)...);
      return;
    }
    if (end_ + 1 > data_[first_arr_ + (n_arrays_ - 1)] + (S - 1)) {
      if (data_[first_arr_ + (n_arrays_ - 1)] == data_[main_capacity_ - 1]) {
        auto needed = DivAndCeil(n_arrays_, S);
        auto fresh_arr = external_alloc_traits::allocate(
            external_alloc_, main_capacity_ + needed * 2);
        auto num = DivAndCeil((main_capacity_ + needed * 2 - n_arrays_), 2);

        for (size_t i = 0; i < n_arrays_; ++i) {
          fresh_arr[num + i] = *(data_ + first_arr_ + i);
        }
        first_arr_ = num;
        fresh_arr[first_arr_ + n_arrays_] = alloc_traits::allocate(alloc_, S);
        end_ = fresh_arr[first_arr_ + n_arrays_];
        try {
          alloc_traits::construct(alloc_, end_, std::forward<Args>(args)...);
          ++n_arrays_;
        } catch (...) {
          alloc_traits::deallocate(alloc_, fresh_arr[first_arr_ + n_arrays_],
                                   S);
          external_alloc_traits::deallocate(external_alloc_, fresh_arr,
                                            main_capacity_ + needed * 2);
          ++first_arr_;
          throw;
        }
        external_alloc_traits::deallocate(external_alloc_, data_,
                                          main_capacity_);
        data_ = fresh_arr;
        main_capacity_ = main_capacity_ + needed * 2;
      } else {
        // internal
        data_[first_arr_ + n_arrays_] = alloc_traits::allocate(alloc_, S);
        end_ = data_[first_arr_ + n_arrays_];
        try {
          alloc_traits::construct(alloc_, end_, std::forward<Args>(args)...);
          ++n_arrays_;
        } catch (...) {
          alloc_traits::deallocate(alloc_, data_[first_arr_ + n_arrays_], S);
          end_ = data_[first_arr_ + n_arrays_] + (S - 1);
          throw;
        }
      }
    } else {
      // just push
      try {
        alloc_traits::construct(alloc_, end_ + 1, std::forward<Args>(args)...);
        ++end_;
      } catch (...) {
        throw;
      }
    }
    ++size_;
  }
  void push_back(T&& elem) { emplace_back(std::move(elem)); }
  void push_front(T&& elem) { emplace_front(std::move(elem)); }
  void pop_front() {
    alloc_traits::destroy(alloc_, start_);
    if (start_ + 1 > data_[first_arr_] + (S - 1)) {
      // delete array
      alloc_traits::deallocate(alloc_, data_[first_arr_], S);
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
    alloc_traits::destroy(alloc_, end_);
    --size_;
    if (end_ - 1 < data_[first_arr_ + (n_arrays_ - 1)]) {
      // delete array
      alloc_traits::deallocate(alloc_, data_[first_arr_ + (n_arrays_ - 1)], S);
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
      auto fresh_deque = Deque<T, Allocator>();
      while (it1 != iter) {
        fresh_deque.push_back(*it1);
        ++it1;
      }
      fresh_deque.push_back(elem);
      while (it1 != it2) {
        fresh_deque.push_back(*it1);
        ++it1;
      }
      *this = fresh_deque;
    }
  }
  void erase(iterator iter) {
    auto it1 = begin();
    auto it2 = end();

    auto fresh_deque = Deque<T, Allocator>();
    while (it1 != iter) {
      fresh_deque.push_back(*it1);
      ++it1;
    }
    ++it1;
    while (it1 != it2) {
      fresh_deque.push_back(*it1);
      ++it1;
    }

    *this = fresh_deque;
  }
  void print() {
    auto iter = begin();
    while (iter != end()) {
      std::cout << *iter;
      ++iter;
    }
    std::cout << "\n";
  }
  template <typename... Args>
  void emplace(iterator iter, Args&&... args) {
    auto it1 = begin();
    auto it2 = end();
    if (iter <= it2) {
      auto fresh_deque = Deque<T, Allocator>();
      while (it1 != iter) {
        fresh_deque.emplace_back(*it1);
        ++it1;
      }
      fresh_deque.emplace_back(args...);
      while (it1 != it2) {
        fresh_deque.emplace_back(*it1);
        ++it1;
      }
      *this = fresh_deque;
    }
  }
  ~Deque() {
    if (!empty()) {
      auto iter = begin();
      while (iter != end()) {
        auto external_num = iter.ptr_ - data_;
        alloc_traits::destroy(alloc_, data_[external_num] + iter.number_);
        ++iter;
      }
      for (size_t i = 0; i < n_arrays_; ++i) {
        alloc_traits::deallocate(alloc_, data_[first_arr_ + i], S);
      }
    }
    external_alloc_traits::deallocate(external_alloc_, data_, main_capacity_);
  }
};

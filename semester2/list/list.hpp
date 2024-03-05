#include <iostream>
#include <string>

size_t Inv(size_t n) {
  if (n == 0) {
    return 1;
  }
  return 0;
}

template <typename T, typename Allocator = std::allocator<T>>
class List {
 private:
  struct BaseNode {
   public:
    BaseNode* prev = this;
    BaseNode* next = this;
    BaseNode() = default;
    BaseNode(BaseNode* prev, BaseNode* next) : prev(prev), next(next) {}
    BaseNode* get_connected(size_t n) {
      if (n == 0) {
        return prev;
      }
      return next;
    }
    void set_connected(size_t n, const BaseNode* value) {
      if (n == 0) {
        prev = const_cast<BaseNode*>(value);
      } else {
        next = const_cast<BaseNode*>(value);
      }
    }
  };
  struct Node : public BaseNode {
    T value;
    Node() = default;
    Node(const T& elem, Node* prev = nullptr, Node* next = nullptr)
        : BaseNode(prev, next), value(elem) {}
  };

  template <bool IsConst>
  class CommonIterator {
   private:
    Node* ptr_;

   public:
    using iterator_category =
        std::bidirectional_iterator_tag;  // type of our iterator
    using difference_type = std::ptrdiff_t;
    using value_type = std::conditional_t<IsConst, const T, T>;
    using pointer_n =
        std::conditional_t<IsConst, const Node*, Node*>;  // or value_type*
    // using double_pointer = std::conditional_t<IsConst, const T**, T**>;
    using reference =
        std::conditional_t<IsConst, const T&, T&>;  // or value_type&
    using pointer =
        std::conditional_t<IsConst, const T*, T*>;  // or value_type&

    CommonIterator(pointer_n ptr) : ptr_(ptr) {}
    reference operator*() const { return (ptr_)->value; }
    pointer operator->() const { return &(ptr_->value); }
    CommonIterator& operator++() {
      ptr_ = static_cast<Node*>(ptr_->next);
      return *this;
    }
    CommonIterator& operator--() {
      ptr_ = static_cast<Node*>(ptr_->prev);
      return *this;
    }
    CommonIterator operator++(int) {
      auto old = ptr_;
      ptr_ = static_cast<Node*>(ptr_->next);
      return old;
    }
    CommonIterator operator--(int) {
      auto old = ptr_;
      ptr_ = static_cast<Node*>(ptr_->prev);
      return old;
    }
    CommonIterator arithmetic(size_t n) {
      auto old = ptr_;
      ptr_ = static_cast<Node*>(ptr_->get_connected(n));
      return old;
    }
    template <bool F>
    bool operator==(const CommonIterator<F>& other) const {
      return ptr_ == other.ptr_;
    }
    template <bool B>
    bool operator!=(const CommonIterator<B>& other) const {
      return !(*this == other);
    }
  };
  using n_alloc =
      typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
  using alloc_traits = std::allocator_traits<n_alloc>;
  n_alloc alloc_;
  BaseNode fake_n_ = BaseNode();
  size_t size_ = 0;

 public:
  using value_type = T;
  using allocator_type = Allocator;
  List() = default;
  void clean(size_t how_much, Node* current, Node* prev) {
    Node* tmp = nullptr;
    for (size_t j = 0; j < how_much; ++j) {
      tmp = static_cast<Node*>(current->prev);
      if (j == 0) {
        alloc_traits::deallocate(alloc_, current, 1);
        current = prev;
      } else {
        alloc_traits::deallocate(alloc_, current, 1);
        alloc_traits::destroy(alloc_, current);
        current = static_cast<Node*>(tmp);
      }
    }
  };
  List(size_t count, const T& value, const Allocator& alloc = Allocator())
      : alloc_(alloc) {
    Node* current = static_cast<Node*>(&fake_n_);
    Node* prev = current;
    for (size_t i = 0; i < count; ++i) {
      current = alloc_traits::allocate(alloc_, 1);
      try {
        alloc_traits::construct(alloc_, current, value);
        current->prev = prev;
        prev->next = current;
        prev = current;
      } catch (...) {
        clean(i + 1, current, prev);
        throw;
      }
    }
    size_ = count;
    current->next = static_cast<Node*>(&fake_n_);
    fake_n_.prev = current;
  }
  explicit List(size_t count, const Allocator& alloc = Allocator())
      : alloc_(alloc) {
    Node* current = static_cast<Node*>(&fake_n_);
    Node* prev = current;
    for (size_t i = 0; i < count; ++i) {
      current = alloc_traits::allocate(alloc_, 1);
      try {
        alloc_traits::construct(alloc_, current);
        current->prev = prev;
        prev->next = current;
        prev = current;
      } catch (...) {
        clean(i + 1, current, prev);
        throw;
      }
    }
    size_ = count;
    current->next = static_cast<Node*>(&fake_n_);
    fake_n_.prev = current;
  }
  List(const List& other, bool full_copy = false)
      : fake_n_(other.fake_n_), size_(other.size()) {
    alloc_ = other.get_allocator();
    if (!full_copy) {
      alloc_ = std::allocator_traits<decltype(other.alloc_)>::
          select_on_container_copy_construction(other.alloc_);
    }
    auto start = other.begin();
    auto beg = start;
    auto end = other.end();
    size_t ind = 0;

    Node* current = static_cast<Node*>(&fake_n_);
    Node* prev = current;
    Node* tmp = nullptr;
    while (start != end) {
      current = alloc_traits::allocate(alloc_, 1);
      try {
        alloc_traits::construct(alloc_, current, *start);
        current->prev = prev;
        prev->next = current;
        prev = current;
      } catch (...) {
        ++start;
        while (start != beg) {
          if (ind == 0) {
            alloc_traits::deallocate(alloc_, current, 1);
            current = prev;
          } else {
            tmp = static_cast<Node*>(current->prev);
            alloc_traits::destroy(alloc_, current);
            alloc_traits::deallocate(alloc_, current, 1);
            current = static_cast<Node*>(tmp);
          }
          --start;
          ++ind;
        }
        throw;
      }
      ++start;
    }
    current->next = static_cast<Node*>(&fake_n_);
    fake_n_.prev = current;
  }
  void swap(List& other) {
    std::swap(size_, other.size_);
    Node* old_fake_next = static_cast<Node*>(fake_n_.next);
    Node* old_fake_prev = static_cast<Node*>(fake_n_.prev);

    fake_n_.next = other.fake_n_.next;
    fake_n_.prev = other.fake_n_.prev;
    (other.fake_n_.next)->prev = &fake_n_;
    (other.fake_n_.prev)->next = &fake_n_;

    if (other.size_ > 0) {
      other.fake_n_.next = old_fake_next;
      other.fake_n_.prev = old_fake_prev;
      old_fake_next->prev = &other.fake_n_;
      old_fake_prev->next = &other.fake_n_;
    } else {
      other.fake_n_.next = &other.fake_n_;
      other.fake_n_.prev = &other.fake_n_;
    }
    if (std::allocator_traits<
            Allocator>::propagate_on_container_copy_assignment::value &&
        alloc_ != other.alloc_) {
      alloc_ = other.alloc_;
    }
  }
  List& operator=(const List& other) {
    auto current = static_cast<Node*>(fake_n_.next);

    List<T, Allocator> copy(other, true);
    this->swap(copy);
    return *this;
  }
  List(std::initializer_list<T> init, const Allocator& alloc = Allocator())
      : alloc_(alloc) {
    auto start = init.begin();

    Node* current = static_cast<Node*>(&fake_n_);
    Node* prev = current;
    while (start != init.end()) {
      current = alloc_traits::allocate(alloc_, 1);
      try {
        alloc_traits::construct(alloc_, current, *start);
        current->prev = prev;
        prev->next = current;
        prev = current;
        ++start;
      } catch (...) {
        while (start != init.begin()) {
          alloc_traits::destroy(alloc_, current);
          alloc_traits::deallocate(alloc_, current, 1);
          current = static_cast<Node*>(current->prev);
          prev = static_cast<Node*>(current->prev);
          --start;
        }
        throw;
      }
    }
    size_ = init.size();
    current->next = static_cast<Node*>(&fake_n_);
    fake_n_.prev = current;
  }
  void insert(size_t pos, const T& elem) {
    auto new_n = alloc_traits::allocate(alloc_, 1);
    try {
      alloc_traits::construct(alloc_, new_n, elem);
      static_cast<Node*>(fake_n_.get_connected(pos))
          ->set_connected(Inv(pos), new_n);
      new_n->set_connected(pos, fake_n_.get_connected(pos));
      new_n->set_connected(Inv(pos), static_cast<Node*>(&fake_n_));
      fake_n_.set_connected(pos, new_n);
      ++size_;
    } catch (...) {
      alloc_traits::destroy(alloc_, new_n);
      alloc_traits::deallocate(alloc_, new_n, 1);
      throw;
    }
  }
  void push_front(const T& elem) { insert(1, elem); }
  void push_back(const T& elem) { insert(0, elem); }
  void erase(size_t pos) {
    if (size() > 0) {
      auto tmp = static_cast<Node*>(fake_n_.get_connected(pos));
      (tmp->get_connected(pos))
          ->set_connected(Inv(pos), static_cast<Node*>(&fake_n_));
      fake_n_.set_connected(pos, tmp->get_connected(pos));
      alloc_traits::destroy(alloc_, tmp);
      alloc_traits::deallocate(alloc_, tmp, 1);
      --size_;
    }
  }
  void pop_front() { erase(1); }
  void pop_back() { erase(0); }
  n_alloc get_allocator() const { return alloc_; }
  size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }
  T& front() { return *begin(); }
  T& back() {
    auto tmp = end();
    --tmp;
    return *tmp;
  }
  ~List() {
    fake_n_.~BaseNode();
    auto current = static_cast<Node*>(fake_n_.next);
    Node* tmp = nullptr;
    for (size_t i = 0; i < size() + 1; ++i) {
      tmp = static_cast<Node*>(current->prev);
      if (current->prev != &fake_n_) {
        alloc_traits::destroy(alloc_, tmp);
        alloc_traits::deallocate(alloc_, tmp, 1);
      }
      current = static_cast<Node*>(current->next);
    }
  }
  using iterator = CommonIterator<false>;
  using const_iterator = CommonIterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() {
    auto tmp = static_cast<Node*>(fake_n_.next);
    return iterator(tmp);
  }
  iterator end() {
    auto tmp = static_cast<Node*>(&fake_n_);
    return iterator(tmp);
  }
  iterator begin() const {
    auto tmp = static_cast<Node*>(const_cast<BaseNode*>(&fake_n_)->next);
    return iterator(tmp);
  }
  iterator end() const {
    auto tmp = static_cast<Node*>(const_cast<BaseNode*>(&fake_n_));
    return iterator(tmp);
  }
  const_iterator cbegin() const { return const_iterator(begin()); }
  const_iterator cend() const { return const_iterator(end()); }
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
  reverse_iterator rend() const { return const_reverse_iterator(begin()); }
};

#include <concepts>
#include <iostream>
#include <memory>

struct BaseControlBlock {
  int shared_count = 1;
  int weak_count = 0;
  virtual void use_deleter() = 0;
  virtual void dealloc(BaseControlBlock* block) = 0;
  virtual void* get_pointer() noexcept = 0;

  virtual bool is_regular() noexcept = 0;
  virtual ~BaseControlBlock() = default;
  void incriment_shared(int how_much) { shared_count += how_much; };
  BaseControlBlock() = default;
  BaseControlBlock(int shared_count, int weak_count)
      : shared_count(shared_count), weak_count(weak_count) {}
};

template <typename T, typename Deleter, typename Alloc = std::allocator<T>>
struct ControlBlockRegular : BaseControlBlock {
  T* ptr = nullptr;
  Deleter deleter;
  Alloc alloc;

  virtual void use_deleter() override { deleter(ptr); }
  virtual void dealloc(BaseControlBlock* block) override {
    using alloc_block = typename std::allocator_traits<
        Alloc>::template rebind_alloc<ControlBlockRegular<T, Deleter, Alloc>>;
    using alloc_traits = std::allocator_traits<alloc_block>;
    alloc_block new_alloc = alloc;

    alloc_traits::destroy(
        new_alloc,
        reinterpret_cast<ControlBlockRegular<T, Deleter, Alloc>*>(block));
    alloc_traits::deallocate(
        new_alloc,
        reinterpret_cast<ControlBlockRegular<T, Deleter, Alloc>*>(block), 1);
  }
  virtual void* get_pointer() noexcept override { return ptr; }
  virtual bool is_regular() noexcept override { return true; }
  ControlBlockRegular() = default;
  ControlBlockRegular(int shared_count, int weak_count, Deleter deleter,
                      Alloc alloc, T* ptr)
      : ptr(ptr),
        BaseControlBlock(shared_count, weak_count),
        deleter(deleter),
        alloc(alloc) {}
  ~ControlBlockRegular() {
    deleter.~Deleter();
    alloc.~Alloc();
  }
};
template <typename T, typename Alloc = std::allocator<T>>
struct ControlBlockMakeShared : BaseControlBlock {
  Alloc alloc;
  T object;

  ControlBlockMakeShared() = default;
  template <typename AAlloc, typename... Args>
  ControlBlockMakeShared(int shared_count, int weak_count, AAlloc alloc,
                         Args&&... args)
      : BaseControlBlock(shared_count, weak_count),
        alloc(alloc),
        object(T(std::forward<Args>(args)...)) {}
  virtual void use_deleter() override { object.~T(); }
  virtual void dealloc(BaseControlBlock* block) override {
    using alloc_block = typename std::allocator_traits<
        Alloc>::template rebind_alloc<ControlBlockMakeShared<T, Alloc>>;
    using alloc_dummy =
        typename std::allocator_traits<Alloc>::template rebind_alloc<Alloc>;
    using alloc_traits = std::allocator_traits<alloc_block>;
    using dummy_traits = std::allocator_traits<alloc_dummy>;
    alloc_block new_alloc = alloc;
    alloc_dummy dummy_alloc = alloc;

    dummy_traits::destroy(dummy_alloc, &alloc);
    alloc_traits::deallocate(
        new_alloc, reinterpret_cast<ControlBlockMakeShared<T, Alloc>*>(block),
        1);
  }
  virtual void* get_pointer() noexcept override { return &object; }
  virtual bool is_regular() noexcept override { return false; }
};

template <typename T, typename Y>
concept Compatible = std::is_convertible_v<Y*, T*> || std::same_as<T, Y>;

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

template <typename T>
class EnableSharedFromThis;

template <typename T>
class SharedPtr {
  template <typename Y, typename... Args>
  friend SharedPtr<Y> MakeShared(Args&&... args);

  template <typename Y, typename Alloc, typename... Args>
  friend SharedPtr<Y> AllocateShared(const Alloc& alloc, Args&&... args);

  template <typename Y>
  friend class SharedPtr;

  template <typename Y>
  friend class WeakPtr;

 public:
  using basic_deleter = decltype(std::default_delete<T>());
  using basic_alloc = decltype(std::allocator<T>());

 private:
  BaseControlBlock* cb_ = nullptr;
  SharedPtr(BaseControlBlock* ptr_cb, bool from_weak = false) : cb_(ptr_cb) {
    if (from_weak) {
      ++(cb_->shared_count);
    }
  }

 public:
  SharedPtr() : cb_(nullptr) {}

  SharedPtr(std::nullptr_t) : cb_(nullptr) {}

  template <typename Y>
  requires Compatible<T, Y> SharedPtr(Y* ptr)
      : cb_(new ControlBlockRegular<T, basic_deleter, basic_alloc>(
            1, 0, basic_deleter(), basic_alloc(), ptr)) {
    if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
      ptr->wptr_ = *this;
    }
  }

  template <typename Y, typename Deleter>
  requires Compatible<T, Y> SharedPtr(Y* ptr, Deleter deleter)
      : cb_(new ControlBlockRegular<T, Deleter, basic_alloc>{
            1, 0, deleter, basic_alloc(), ptr}) {}
  template <typename Y, typename Deleter, typename Alloc>
  requires Compatible<T, Y> SharedPtr(Y* ptr, Deleter deleter, Alloc alloc) {
    using alloc_control_block = typename std::allocator_traits<
        Alloc>::template rebind_alloc<ControlBlockRegular<T, Deleter, Alloc>>;
    using alloc_traits = std::allocator_traits<alloc_control_block>;
    alloc_control_block new_alloc = alloc;
    auto block = alloc_traits::allocate(new_alloc, 1);
    alloc_traits::construct(new_alloc, block, 1, 0, deleter, alloc, ptr);

    cb_ = block;
  }
  template <typename Y>
  requires std::is_base_of_v<T, Y> SharedPtr(const SharedPtr<Y>& other)
      : cb_(other.cb_) {
    ++(cb_->shared_count);
  }
  SharedPtr(const SharedPtr& other) : cb_(other.cb_) {
    if (cb_ == nullptr) {
      return;
    }
    ++(cb_->shared_count);
  }

  template <typename Y>
  requires std::is_base_of_v<T, Y> SharedPtr(SharedPtr<T>&& other)
      : cb_(other.cb_) {
    other.cb = nullptr;
  }
  SharedPtr(SharedPtr&& other) : cb_(other.cb_) { other.cb_ = nullptr; }

  template <typename Y>
  requires std::is_base_of_v<T, Y> SharedPtr& operator=(
      const SharedPtr<Y>& other) {
    if (cb_ == other.cb_) {
      return *this;
    }
    handle_cb();
    cb_ = other.cb_;
    ++(cb_->shared_count);
    return *this;
  }
  SharedPtr& operator=(const SharedPtr& other) noexcept {
    if (cb_ == other.cb_) {
      return *this;
    }
    handle_cb();
    cb_ = other.cb_;
    ++(cb_->shared_count);
    return *this;
  }
  template <typename Y>
  requires std::is_base_of_v<T, Y> SharedPtr& operator=(SharedPtr<Y>&& other) {
    handle_cb();
    cb_ = other.cb_;
    other.cb_ = nullptr;
    return *this;
  }
  SharedPtr& operator=(SharedPtr&& other) {
    if (cb_ == other.cb_) {
      return *this;
    }
    handle_cb();
    cb_ = other.cb_;
    other.cb_ = nullptr;
    return *this;
  }
  size_t use_count() const noexcept {
    if (cb_ == nullptr) {
      return 0;
    }
    return cb_->shared_count;
  }
  T& operator*() const { return *(reinterpret_cast<T*>(cb_->get_pointer())); }
  T* operator->() const { return reinterpret_cast<T*>(cb_->get_pointer()); }
  T* get() const {
    if (cb_ == nullptr) {
      return nullptr;
    }
    return reinterpret_cast<T*>(cb_->get_pointer());
  }
  void reset() noexcept {
    handle_cb();
    cb_ = nullptr;
  }
  void handle_cb() {
    if (cb_ == nullptr) {
      return;
    }

    --cb_->shared_count;
    if (cb_->shared_count == 0) {
      cb_->use_deleter();
      if (cb_->weak_count == 0) {
        cb_->dealloc(cb_);
      }
    }
  }
  ~SharedPtr() { handle_cb(); }
};

template <typename T>
class WeakPtr {
  template <typename Y>
  friend class EnableSharedFromThis;

 private:
  BaseControlBlock* cb_ = nullptr;

 public:
  WeakPtr() = default;
  WeakPtr(std::nullptr_t) : cb_(nullptr) {}
  WeakPtr(const WeakPtr& other) noexcept : cb_(other.cb_) {
    ++(cb_->weak_count);
  }
  template <typename Y>
  requires Compatible<T, Y> WeakPtr(const WeakPtr<Y>& other)
  noexcept : cb_(other.cb_) { ++(cb_->weak_count); }
  template <typename Y>
  requires Compatible<T, Y> WeakPtr(const SharedPtr<Y>& other)
  noexcept : cb_(other.cb_) { ++(cb_->weak_count); }
  WeakPtr(WeakPtr&& other) noexcept : cb_(other.cb_) { other.cb_ = nullptr; }

  template <typename Y>
  requires Compatible<T, Y> WeakPtr(WeakPtr<Y>&& other)
  noexcept : cb_(other.cb_) { other.cb_ = nullptr; }

  WeakPtr& operator=(const WeakPtr& other) {
    handle_cb(true);
    cb_ = other.cb_;
    ++(cb_->weak_count);
    return *this;
  }
  template <typename Y>
  WeakPtr& operator=(const WeakPtr<Y>& other) {
    handle_cb(true);
    cb_ = other.cb_;
    ++(cb_->weak_count);
    return *this;
  }
  WeakPtr& operator=(WeakPtr&& other) {
    handle_cb(false);
    cb_ = other.cb_;
    other.cb_ = nullptr;
    return *this;
  }
  template <typename Y>
  WeakPtr& operator=(WeakPtr<Y>&& other) {
    handle_cb(false);
    cb_ = other.cb_;
    other.cb_ = nullptr;
    return *this;
  }
  size_t use_count() const noexcept { return cb_->shared_count; }
  void handle_cb(bool decrement) {
    if (cb_ == nullptr) {
      return;
    }
    if (decrement) {
      --(cb_->weak_count);
    };
    if (cb_->shared_count == 0 && cb_->weak_count == 0) {
      cb_->dealloc(cb_);
    }
  }

  void reset() {
    handle_cb();
    cb_ = nullptr;
  }
  bool expired() const noexcept {
    if (cb_ == nullptr) {
      return false;
    }
    return (cb_->shared_count == 0);
  }
  SharedPtr<T> lock() const {
    if (expired()) {
      return SharedPtr<T>();
    }
    return SharedPtr<T>(cb_, true);
  }
  ~WeakPtr() { handle_cb(true); }
};

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
  using alloc = decltype(std::allocator<T>());
  using control_block = ControlBlockMakeShared<T, alloc>;
  auto block =
      new control_block(1, 0, std::allocator<T>(), std::forward<Args>(args)...);
  return SharedPtr<T>(block, false);
}

template <typename T, typename Alloc, typename... Args>
SharedPtr<T> AllocateShared(const Alloc& alloc, Args&&... args) {
  using alloc_control_block = typename std::allocator_traits<
      Alloc>::template rebind_alloc<ControlBlockMakeShared<T, Alloc>>;
  using alloc_traits = std::allocator_traits<alloc_control_block>;

  alloc_control_block new_alloc = alloc;
  auto block = alloc_traits::allocate(new_alloc, 1);
  alloc_traits::construct(new_alloc, block, 1, 0, alloc,
                          std::forward<Args>(args)...);
  return SharedPtr<T>(block, false);
}

template <typename T>
class EnableSharedFromThis {
  template <typename Y>
  friend class SharedPtr;

 private:
  WeakPtr<T> wptr_ = nullptr;

 protected:
  SharedPtr<T> shared_from_this() const {
    if (wptr_.cb_ == nullptr || wptr_.expired()) {
      throw std::exception();
    }
    return wptr_.lock();
  }
};

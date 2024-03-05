#include "big_integer.hpp"

#include <iostream>
#include <vector>

template <typename T>

void SwapVars(T& var1, T& var2) {
  T tmp = var1;
  var1 = var2;
  var2 = tmp;
}
BigInteger::BigInteger(int64_t number) : BigInteger(std::to_string(number)) {}
BigInteger::BigInteger(std::string number)
    : length_real_(number.size()), length_nominal_(number.size()) {
  this->number_.resize(number.size());
  int i = 0;
  const int kKTransform = 0;
  if (number[0] == '-') {
    i = 1;
    this->sign_ = -1;
  }
  for (; i < length_real_; ++i) {
    this->number_[i] = (number[i] - kKTransform);
  }
  if (sign_ < 0) {
    --length_nominal_;
  }
  std::cout << "\n";
}

BigInteger& BigInteger::operator=(BigInteger& other) {
  this->Swap(other);
  return *this;
}

BigInteger& BigInteger::operator=(const BigInteger& other) {
  length_real_ = other.length_real_;
  length_nominal_ = other.length_nominal_;
  number_ = other.number_;
  sign_ = other.sign_;
  return *this;
}

BigInteger::BigInteger(const BigInteger& other) {
  this->number_ = other.number_;
  sign_ = other.sign_;
  length_real_ = other.length_real_;
  length_nominal_ = other.length_nominal_;
};
void BigInteger::PrintNum() const {
  int i = 0;
  if (sign_ < 0) {
    i = 1;
    std::cout << "-";
  }
  for (; i < this->length_real_; ++i) {
    std::cout << number_[i];
  }
  std::cout << "len: " << length_nominal_;
  std::cout << "\n";
}

void BigInteger::Swap(BigInteger& num) {
  SwapVars(this->number_, num.number_);
  SwapVars(sign_, num.sign_);
  SwapVars(length_real_, num.length_real_);
  SwapVars(length_nominal_, num.length_nominal_);
}
bool BigInteger::IsValid(int index) const {
  bool res = false;
  if (index >= 0 && index <= length_real_ - 1) {
    res = true;
  }
  return res;
}
void BigInteger::ChainAdd(int num, int index) {
  int tmp = number_[index] + num;
  const int kKTen = 10;
  const int kKUpperBound = 9;

  if (index < 0 || index < length_nominal_ - length_real_ ||
      index > length_real_ - 1 || (tmp >= 0 && tmp <= kKUpperBound)) {
    exit;
  }
  number_[index] += num;
  if (tmp < 0 && sign_ > 0 && IsValid(index - 1)) {
    ChainAdd(num, index - 1);

  } else if (tmp > kKUpperBound && IsValid(index - 1)) {
    number_[index] -= kKTen;
    ChainAdd(1, index - 1);
  }
}
void BigInteger::ClearZeros() {
  std::vector<int> clean = {};

  bool zero = true;
  for (int i = 0; i < number_.size(); ++i) {
    if (number_[i] != 0) {
      zero = false;
    }
    if (!zero) {
      clean.push_back(number_[i]);
    }
  }
  number_ = clean;
  length_real_ = clean.size();
  length_nominal_ = length_real_;
  if (sign_ < 0) {
    ++length_nominal_;
  }
}
void BigInteger::Transform() {
  int i = 0;
  if (sign_ < 0) {
    i = 1;
  }
  for (; i < number_.size(); i++) {
    number_[i] *= sign_;
  }
}

BigInteger& BigInteger::operator+=(const BigInteger& other) {
  int tmp = 0;
  int max_capacity = std::max(number_.size(), other.number_.size());
  number_.resize(max_capacity);
  Transform();
  // other.Transform();
  for (int i = 0; i < std::min(length_nominal_, other.length_nominal_); ++i) {
    ChainAdd(other.number_[other.length_real_ - i - 1] * other.sign_,
             length_real_ - i - 1);
  }
  // ClearZeros();
  length_nominal_ = std::max(number_.size(), other.length_nominal_);
  length_real_ = length_nominal_;
  return *this;
}

BigInteger operator+(const BigInteger& first, const BigInteger& last) {
  BigInteger copy = first;
  first.PrintNum();
  copy += last;
  copy.PrintNum();
  return copy;
}

BigInteger operator-(const BigInteger& first, const BigInteger& last) {
  BigInteger copy = first;
  copy += last;
  return copy;
}

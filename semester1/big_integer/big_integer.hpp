#pragma once
#include <string>
#include <vector>

class BigInteger {
 private:
  std::vector<int> number_;
  size_t length_real_ = 0;
  size_t length_nominal_ = 0;
  int sign_ = 1;

 public:
  //~BigInteger();
  BigInteger& operator=(BigInteger& other);
  BigInteger& operator=(const BigInteger& other);
  BigInteger();
  BigInteger(std::string number);
  BigInteger(int64_t number);
  BigInteger(const BigInteger& other);

  void PrintNum() const;
  void Swap(BigInteger& num);
  void ChainAdd(int num, int index);
  bool IsValid(int index) const;
  void ClearZeros();
  void Transform();

  BigInteger& operator+=(const BigInteger& other);
};
BigInteger operator+(const BigInteger& first, const BigInteger& last);
bool operator==(const BigInteger& first, const BigInteger& last);
bool operator!=(const BigInteger& first, const BigInteger& last);
bool operator>(const BigInteger& first, const BigInteger& last);
bool operator>=(const BigInteger& first, const BigInteger& last);
bool operator<(const BigInteger& first, const BigInteger& last);
bool operator<=(const BigInteger& first, const BigInteger& last);

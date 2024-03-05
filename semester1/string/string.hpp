#pragma once
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <vector>

class String {
 private:
  size_t size_ = 0;
  size_t capacity_ = 0;
  char* string_ = new char[2];

 public:
  String();
  String(size_t size, char character);
  String(const char* str);
  String(const String& str);
  String& operator=(String& other);
  String& operator=(String const& other);
  //копирующий оператор присваивания
  ~String();

  void PushBack(char character);
  void PopBack();
  bool Empty() const;
  void Clear();
  size_t Size() const;
  size_t Capacity() const;
  char& Front();
  const char& Front() const;
  char& Back();
  const char& Back() const;

  const char* Data() const;
  char* Data();
  void Swap(String& str);
  void Resize(size_t new_size);
  void Resize(size_t new_size, char character);
  void Reserve(size_t new_cap);
  void ShrinkToFit();
  std::vector<String> Split(const String& delim = " ");
  String Join(const std::vector<String>& strings) const;

  char& operator[](size_t index);
  const char& operator[](size_t index) const;

  String& operator+=(const String& other);
  String& operator+=(const char& character);
  friend String operator*(String first, size_t n);
  String& operator*=(size_t n);

  void AddNul();
  void PrintStr();
};
String operator+(const String& first, const String& last);
bool operator==(const String& first, const String& last);
bool operator!=(const String& first, const String& last);
bool operator>(const String& first, const String& last);
bool operator>=(const String& first, const String& last);
bool operator<(const String& first, const String& last);
bool operator<=(const String& first, const String& last);

std::ostream& operator<<(std::ostream& out, const String& str);
std::istream& operator>>(std::istream& in_stream, String& str);
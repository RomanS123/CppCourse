#include "string.hpp"

#include <stdlib.h>

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <random>

String::String() = default;
String::String(size_t size, char character = '\0')
    : size_(size), capacity_(size_), string_(new char[capacity_ + 1]) {
  memset(string_, character, size_);
  String::AddNul();
}
String::String(const char* str) : size_(strlen(str)), capacity_(size_) {
  delete[] string_;
  string_ = new char[capacity_ + 1];
  memcpy(string_, str, size_);
  AddNul();
}
//копирующий конструктор
String::String(const String& str) : String(str.size_, '\0') {
  memcpy(string_, str.string_, str.size_);
  string_[size_] = '\0';
}

//копирующий оператор присваивания
String& String::operator=(String& other) {
  this->Swap(other);
  return *this;
}
String& String::operator=(const String& other) {
  delete[] string_;
  size_ = other.size_;
  capacity_ = other.capacity_;
  string_ = new char[other.capacity_];
  memcpy(string_, other.string_, other.capacity_);
  return *this;
}
//деструктор
String::~String() { delete[] string_; }

void String::PushBack(char character) {
  if (capacity_ == 0) {
    capacity_ = 1;
  }
  size_t size_tmp = size_;
  if (size_ >= capacity_ || size_ == 0) {
    Resize(capacity_ * 2);
  }
  string_[size_tmp] = character;
  size_ = ++size_tmp;

  string_[size_tmp] = '\0';
}
void String::PopBack() {
  if (size_ > 0) {
    string_[size_] = ' ';
    --size_;
    AddNul();
  }
}
void String::Clear() { size_ = 0; }

void String::Swap(String& str) {
  std::swap(size_, str.size_);
  std::swap(capacity_, str.capacity_);
  std::swap(string_, str.string_);
}

void String::Resize(size_t new_size) {
  if (new_size > capacity_) {
    char* new_string = new char[new_size + 1];
    memcpy(new_string, string_, size_);
    delete[] string_;
    string_ = new_string;
    if (new_size == 0) {
      new_size = 1;
    }
  }
  size_ = new_size;
  capacity_ = new_size;
  AddNul();
}
void String::Resize(size_t new_size, char character) {
  Reserve(new_size);
  if (new_size > size_) {
    memset(string_ + size_, character, (new_size - size_) * sizeof(character));
  }
  size_ = new_size;
}
void String::Reserve(size_t new_cap) {
  size_t size_tmp = size_;
  if (new_cap > capacity_) {
    Resize(new_cap);
  }
  size_ = size_tmp;
}
void String::ShrinkToFit() {
  if (capacity_ > size_) {
    Resize(size_);
  }
}

char& String::operator[](size_t index) { return *(string_ + index); }
const char& String::operator[](size_t index) const {
  return *(string_ + index);
}
bool String::Empty() const { return this->size_ == 0; }
size_t String::Size() const { return this->size_; }
size_t String::Capacity() const { return this->capacity_; }
const char* String::Data() const { return this->string_; }
char* String::Data() { return this->string_; }

char& String::Front() { return string_[0]; };
const char& String::Front() const { return string_[0]; };
char& String::Back() { return string_[size_ - 1]; };
const char& String::Back() const { return string_[size_ - 1]; };

String& String::operator+=(const String& other) {
  size_t other_size = other.Size();
  for (size_t i = 0; i < other_size; ++i) {
    this->PushBack(other[i]);
  }
  return *this;
};
String& String::operator+=(const char& character) {
  PushBack(character);
  return *this;
}
String operator+(const String& first, const String& last) {
  String copy = first;
  copy += last;
  return copy;
}
String operator*(String first, size_t n) {
  String copy = first;
  first.Reserve(first.Size() * n + 1);
  for (size_t i = 1; i < n; ++i) {
    first += copy;
  }
  if (n == 0) {
    return String(0, ' ');
  }
  first.AddNul();
  // first.size_ += (n - 1) * first.Size();
  return first;
}
String& String::operator*=(size_t n) {
  String copy = *this * n;
  Swap(copy);
  return *this;
}
bool operator==(const String& first, const String& last) {
  if (std::max(first.Size(), last.Size()) !=
      std::min(first.Size(), last.Size())) {
    return false;
  }
  for (size_t i = 0; i < first.Size(); ++i) {
    if (first[i] != last[i]) {
      return false;
    }
  }
  return true;
}
bool operator!=(const String& first, const String& last) {
  return !(first == last);
}
String String::Join(const std::vector<String>& strings) const {
  String joined("");
  size_t to_reserve = size_;
  for (size_t i = 0; i < strings.size(); ++i) {
    to_reserve += strings[i].Size();
  }
  joined.Reserve(to_reserve + 1);
  // joined += *this;

  for (size_t i = 0; i < strings.size(); ++i) {
    joined += strings[i];
    if (i != strings.size() - 1) {
      joined += *this;
    }
  }
  return joined;
}
bool Find(const String& str, const String& find) {
  for (size_t i = 0; i < str.Size(); ++i) {
    if (str[i] == find[0]) {
      int cmp = std::strncmp(str.Data() + i, find.Data(), str.Size() - i);
      if (cmp == 0) {
        return true;
      }
    }
  }
  return false;
}
size_t Modulo(size_t first, size_t last) {
  size_t result = 0;
  if (last > first) {
    result = last - first;
  } else {
    result = first - last;
  }
  return result;
}
std::vector<String> String::Split(const String& delim) {
  std::vector<String> result = {};
  size_t delim_len = delim.Size();
  String current(delim_len);
  size_t last_occ = 0;
  size_t index = 0;
  for (size_t start = 0; start <= Size(); ++start) {
    if (start + delim_len <= Size() + 1) {
      std::cout << "a";
      memcpy(current.string_, string_ + start, delim_len);
    } else {
      memcpy(current.string_, string_ + start, 1);
    }
    if (current == delim || string_[start] == String("\0")) {
      char* tmp = new char[Modulo(start, last_occ) + 1];
      index = start - last_occ;
      if (last_occ > start) {
        index = 0;
      }
      memcpy(tmp, string_ + last_occ, index);
      tmp[index] = '\0';
      result.push_back(String(tmp));
      last_occ = start + delim_len;
      delete[] tmp;
    }
    if (!Find(current, delim) && delim_len > 1) {
      start += delim_len - 2;
    }
  }
  return result;
}
bool operator>(const String& first, const String& last) {
  for (size_t i = 0; i < std::max(first.Size(), last.Size()); ++i) {
    if (first[i] > last[i]) {
      return true;
    }
    if (first[i] < last[i]) {
      return false;
    }
  }
  return false;
}
bool operator>=(const String& first, const String& last) {
  return first > last || first == last;
}
bool operator<(const String& first, const String& last) {
  return !(first > last);
}
bool operator<=(const String& first, const String& last) {
  return first < last || first == last;
}
std::ostream& operator<<(std::ostream& out, const String& str) {
  for (size_t i = 0; i < str.Size(); ++i) {
    out << str[i];
  }
  return out;
}
std::istream& operator>>(std::istream& in_stream, String& str) {
  const size_t kBufferSize = 5000;
  char* array_x = new char[kBufferSize];
  in_stream >> array_x;
  String str_new = String(static_cast<const char*>(array_x));
  delete[] array_x;
  str = str_new;
  return in_stream;
}
void String::AddNul() { string_[size_] = '\0'; }
void String::PrintStr() {
  for (size_t i = 0; i < this->size_; ++i) {
    std::cout << this->string_[i];
  }
}

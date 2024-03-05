#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

template <size_t N, size_t M, typename T = int64_t>
class Matrix {
 public:
  Matrix() {
    matrix_.resize(N);
    for (size_t i = 0; i < N; ++i) {
      matrix_[i].resize(M);
    }
  }
  Matrix(std::vector<std::vector<T>> vec) { matrix_ = vec; }
  Matrix(T elem) {
    for (size_t i = 0; i < N; ++i) {
      matrix_.push_back(std::vector<T>(M, elem));
    }
  }
  Matrix<N, M, T>& operator+=(const Matrix<N, M, T>& other) {
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < M; ++j) {
        (*this)(i, j) += other(i, j);
      }
    }
    return *this;
  }
  T& operator()(size_t row, size_t col) { return matrix_[row][col]; }
  const T& operator()(size_t row, size_t col) const {
    return matrix_[row][col];
  }
  std::vector<std::vector<T>> GetData() const { return matrix_; }
  Matrix& operator-=(const Matrix<N, M, T>& other) {
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < M; ++j) {
        (*this)(i, j) -= other(i, j);
      }
    }
    return *this;
  }
  Matrix<M, N, T> Transposed() {
    Matrix<M, N, T> result((0, 0));
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < M; ++j) {
        result(j, i) = (*this)(i, j);
      }
    }
    return result;
  }

 private:
  std::vector<std::vector<T>> matrix_;
  size_t rows_ = N;
  size_t cols_ = M;
};
template <size_t N, size_t M, typename T>
Matrix<N, M, T> operator+(const Matrix<N, M, T>& matrix1,
                          const Matrix<N, M, T>& matrix2) {
  Matrix<N, M, T> copy = matrix1;
  copy += matrix2;
  return copy;
}
template <size_t N, size_t M, typename T>
Matrix<N, M, T> operator-(const Matrix<N, M, T>& matrix1,
                          const Matrix<N, M, T>& matrix2) {
  Matrix<N, M, T> copy = matrix1;
  copy -= matrix2;
  return copy;
}
template <size_t N, size_t M, size_t P, typename T>
Matrix<N, P, T> operator*(const Matrix<N, M, T>& matrix1,
                          const Matrix<M, P, T>& matrix2) {
  Matrix<N, P, T> result;
  bool filled = false;

  for (size_t i = 0; i < N; ++i) {
    // fixed a row
    for (size_t j = 0; j < P; ++j) {
      // multiplying row by a column
      filled = false;
      for (size_t k = 0; k < M; ++k) {
        if (!filled) {
          result(i, j) = matrix1(i, k) * matrix2(k, j);
          filled = true;
        } else {
          result(i, j) += matrix1(i, k) * matrix2(k, j);
        }
      }
    }
  }
  return result;
}
template <size_t N, size_t M, typename T>
Matrix<N, M, T> operator*(const Matrix<N, M, T>& matrix, T elem) {
  Matrix<N, M, T> copy = matrix;
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < M; ++j) {
      copy(i, j) *= elem;
    }
  }
  return copy;
}

template <size_t N, size_t M, typename T>
bool operator==(const Matrix<N, M, T>& matrix1,
                const Matrix<N, M, T>& matrix2) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < M; ++j) {
      if (matrix1(i, j) != matrix2(i, j)) {
        return false;
      }
    }
  }
  return true;
}
template <size_t N, typename T>
class Matrix<N, N, T> {
 public:
  Matrix() {
    matrix_.resize(N);
    for (size_t i = 0; i < N; ++i) {
      matrix_[i].resize(N);
    }
  }
  Matrix(std::vector<std::vector<T>> vec) { matrix_ = vec; }
  Matrix(T elem) : Matrix() {
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < N; ++j) {
        (*this)(i, j) = elem;
      }
    }
  }
  Matrix<N, N, T>& operator+=(const Matrix<N, N, T>& other) {
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < N; ++j) {
        (*this)(i, j) += other(i, j);
      }
    }
    return *this;
  }
  T& operator()(size_t row, size_t col) { return matrix_[row][col]; }
  const T& operator()(size_t row, size_t col) const {
    return matrix_[row][col];
  }
  std::vector<std::vector<T>> GetData() const { return matrix_; }
  Matrix& operator-=(const Matrix<N, N, T>& other) {
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < N; ++j) {
        (*this)(i, j) -= other(i, j);
      }
    }
    return *this;
  }
  Matrix<N, N, T> Transposed() {
    Matrix<N, N, T> result((0, 0));
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < N; ++j) {
        result(j, i) = (*this)(i, j);
      }
    }
    return result;
  }
  T Trace() {
    T tmp;
    bool empty = true;
    for (size_t i = 0; i < N; ++i) {
      if (empty) {
        tmp = (*this)(i, i);
        empty = false;
      } else {
        tmp += (*this)(i, i);
      }
    }
    return tmp;
  }

 private:
  std::vector<std::vector<T>> matrix_;
  size_t rows_ = N;
  size_t cols_ = N;
};

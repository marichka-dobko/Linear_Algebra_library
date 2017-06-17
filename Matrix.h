//
// Created by dzvinka on 25.04.17.
//

#ifndef LINEAR_ALGEBRA_LIBRARY_MATRIX_H
#define LINEAR_ALGEBRA_LIBRARY_MATRIX_H

#include <iostream>
#include <stdexcept>
#include <string>
#include <cassert>
#include <cmath>
#include "Vector.h"
#include <thread>
#include <mutex>
#include <vector>

using namespace std;
//mutex mtx;
template <class T>
class Matrix {
private:

    struct MatrixMemory {
        T *data;
        size_t cols, rows;

        MatrixMemory(size_t r, size_t c) :
                cols(c), rows(r)
        {
            data = new T[cols * rows];
        }

        ~MatrixMemory() // RAII
        {
            delete[] data;
        }

        inline T &operator()(size_t i, size_t j) {
            return *(data + i * cols + j);
        }

        inline const T &operator()(size_t i, size_t j) const
        {
            return *(data + i * cols + j);
        }

        MatrixMemory (const MatrixMemory& other):
                rows(other.rows), cols(other.cols)
        {
            data = new T[other.rows * other.cols];
            copy(other.data, other.data + (rows * cols), data);
        }

        static void swap(MatrixMemory & a, MatrixMemory & b)
        {
            std::swap(a.data, b.data);
            std::swap(a.cols, b.cols);
            std::swap(a.rows, b.rows);
        }

        MatrixMemory& operator=(const MatrixMemory& b)
        {
            MatrixMemory temp(b);
            MatrixMemory::swap(temp, *this);
            return *this;
        }

    } matr_data;


public:

    Matrix(size_t r, size_t c) : matr_data(c, r) {}
#if 0
    void operator = (T* arr[])
    {

        //std::cout << (sizeof(arr) / sizeof(arr[0])) << std::endl;
//        if ((sizeof(arr) / sizeof(arr[0]) ) > matr_data.rows * matr_data.cols) //TODO:FIX
//        {
//            throw std::out_of_range("Too big arr to initialize matrix with");
//        }
        std::copy(matr_data.data, matr_data.data + (matr_data.rows * matr_data.cols), arr);

    }
#endif
    // square matrix only
    Matrix(std::initializer_list<T> ilst) : matr_data(std::sqrt(ilst.size()), std::sqrt(ilst.size()))
    {
        assert(std::sqrt(ilst.size())*std::sqrt(ilst.size())==ilst.size());
        for (auto i = begin(ilst); i != end(ilst); i++)
        {
            matr_data.data[i - begin(ilst)] = *i;
        }

    }

    Matrix& operator= (std::initializer_list<T> ilst)
    {
        if (ilst.size() != matr_data.rows * matr_data.cols)
        {
            throw std::out_of_range("Too big lst to initialize matrix with");
        }

        for (auto i = begin(ilst); i != end(ilst); i++)
        {
            matr_data.data[i - begin(ilst)] = *i;
        }
        //std::copy(matr_data.data, matr_data.data + (matr_data.rows * matr_data.cols), begin(ilst));
        return *this;
    }

    size_t rows() const {
        return matr_data.rows;
    }

    size_t cols() const {
        return matr_data.cols;
    }

    T* const matrToArr()
    {
        return matr_data.data;
    }

    inline T& operator()(size_t i, size_t j) {
        if (i >= matr_data.rows || j >= matr_data.cols) {
            throw std::out_of_range ("Matrix indexes out of range");
        }
        return matr_data(i, j);

    }

    inline const T& operator()(size_t i, size_t j) const {
        if (i >= matr_data.rows || j >= matr_data.cols) {
            throw std::out_of_range ("Matrix indexes out of range");
        }
        return matr_data(i, j);

    }


    friend std::ostream& operator<<(std::ostream& out, const Matrix& val){
        string c = "";
        for (int i = 0; i < val.matr_data.cols; i ++)
        {
            for (int j = 0; j< val.matr_data.rows; j ++){
                c+= std::to_string(val(i, j));
                c += " ";
            }
            c +='\n';
        }
        out << c;
        return out;
    }

    bool operator==(const Matrix &other) const
    {
        if(rows()!=other.rows() || cols()!=other.cols())
            return false;
        for (int i = 0; i < matr_data.cols; i ++)
        {
            for (int j = 0; j< matr_data.rows; j ++){
                if( (*this)(i,j) != other(i,j))
                    return false;
            }
        }
        return true;
    }
    bool operator!=(const Matrix &other) const {
        return !((*this) == other);
    }

    void mtrx_addition(const Matrix other, size_t n){
        for (size_t iter = 0; iter < rows(); iter++) {
            (*this)(iter, n) = (*this)(iter, n) + other(iter, n);
        }
    }



    Matrix& operator+=(const Matrix &other)
    {
        /**
         * Parallel matrix addition (Matrix + Matrix)
         */
        std::vector<std::thread> th;

        size_t nr_threads = cols();
        for (size_t n = 0; n < nr_threads; ++n) {
            th.push_back(std::thread(&Matrix::mtrx_addition, this, other, n));
        }

        for (auto &t : th) {
            t.join();
        }
        return *this;
    }


    void mtrx_subtraction(const Matrix other, size_t n){
        for (size_t iter = 0; iter < rows(); iter++) {
            (*this)(iter, n) = (*this)(iter, n) - other(iter, n);
        }
    }

    Matrix& operator-=(const Matrix &other)
    {
        /**
         * Parallel subtraction (Matrix - Matrix)
         */
        std::vector<std::thread> th;

        size_t nr_threads = cols();
        for (size_t n = 0; n < nr_threads; ++n) {
            th.push_back(std::thread(&Matrix::mtrx_subtraction, this, other, n));
        }

        for (auto &t : th) {
            t.join();
        }
        return *this;
    }

    void sc_addition(double scalar, size_t n) {
        for (size_t iter = 0; iter < rows(); iter++) {
            (*this)(iter, n) = (*this)(iter, n) + scalar;
        }
    }

    Matrix& operator+=(const T &scalar)
    {
        /**
         * Parallel addition of scalar to matrix (Matrix + scalar)
         */
        std::vector<std::thread> th;

        size_t nr_threads = cols();
        for (size_t n = 0; n < nr_threads; ++n) {
            th.push_back(std::thread(&Matrix::sc_addition, this, scalar, n));
        }
        for (auto &t : th) {
            t.join();
        }
        return *this;
    }


    void sc_subtraction(double scalar, size_t n) {
        for (size_t iter = 0; iter < rows(); iter++) {
            (*this)(iter, n) = (*this)(iter, n) - scalar;
        }
    }

    Matrix& operator-=(const T &scalar)
    {
        /**
         * Parallel subtraction (Matrix - scalar)
         */
        std::vector<std::thread> th;

        size_t nr_threads = cols();
        for (size_t n = 0; n < nr_threads; ++n) {
            th.push_back(std::thread(&Matrix::sc_subtraction, this, scalar, n));
        }

        for (auto &t : th) {
            t.join();
        }
        return *this;
    }

    Matrix operator*(const Matrix &other) const {
        assert(cols == other.rows) ;

        const Matrix &self = *this;
        Matrix<int> res(rows(), cols());

        for (size_t i = 0; i < rows(); i++) {
            for (size_t k = 0; k < cols(); k++) {
                for (size_t j = 0; j < other.rows(); j++) {
                    res(i, k) = self(i, j) * other(j, k);
                }
            }
        }
        return res;
    }

    Matrix<T> multiplication(const Matrix<T> &other) const
    {
        /**
        *  Parallel multiplication of two matrices
        */

        const Matrix<T> &self = *this;
        Matrix<T> res(rows(), other.cols());
        thread workingThreads[this->rows()];
        for (int i = 0; i < this->rows(); i++) {
            workingThreads[i] = thread(&Matrix<T>::multthread, this, other, self, &res, i);
        }
        for (int i = 0; i < this->rows(); i++) {
            workingThreads[i].join() ;
        }
        return res;
    }


    void multthread(const Matrix& other, const Matrix &self, Matrix &result, int numbofrow)
    {
        auto index = self.matr_data.data + (numbofrow - 1) * self.cols();
        vector<T> res(self.cols());
        for (int j = 0; j < other.cols(); j++) {
            for (int i = 0; i < other.rows(); i++) {
                res[j] += self(numbofrow, i) * other(i, j);
            }
        }
        //lock_guard<std::mutex> lock(mtx);
        copy(begin(res), end(res), index);

    }

    void sc_product(double scalar, size_t n) {
        for (size_t iter = 0; iter < rows(); iter++) {
            (*this)(iter, n) = (*this)(iter, n) * scalar;
        }
    }

    Matrix& operator*=(const T &scalar)
    {
        /**
         * Parallel multiplication by scalar
         */
        std::vector<std::thread> th;


        size_t nr_threads = cols();
        for (size_t n = 0; n < nr_threads; ++n) {
            th.push_back(std::thread(&Matrix::sc_product, this, scalar, n));
        }

        for (auto &t : th) {
                t.join();
        }
        return *this;
    }

    Matrix &operator- ()
    {
        Matrix &mat = *this;


        for (size_t i=0; i<rows(); i++)
            for (size_t j=0; j<cols(); j++)
                mat(i, j) =  - mat(i, j);
        return mat;
    }

    Matrix &operator+ ()
    {
        return *this;
    }

};

template<typename T>
inline Matrix<T> operator+(Matrix<T> left, const Matrix<T> &other) {
    assert(left.cols() == other.cols() && left.rows() == other.rows());
    return left += other;
}

template<typename T>
inline Matrix<T> operator+(const T &scalar, Matrix<T> other) {
    return other += scalar;
}


template<typename T>
inline Matrix<T> operator+(Matrix<T> left, const T &scalar)
{
    return left += scalar;
}

template<typename T>
inline Matrix<T> operator-(Matrix<T> left, const Matrix<T> &other) {
    assert(left.cols() == other.cols() && left.rows() == other.rows());
    return left -= other;

}

template<typename T>
inline Matrix<T> operator-(Matrix<T> left, const T &scalar)
{
    return left -= scalar;
}

template<typename T>
inline Matrix<T> operator-(const T &scalar, Matrix<T> right)
{
    return -right+=scalar;
}

template<typename T>
inline Matrix<T> operator*(Matrix<T> left, const T& scalar) {
    return left *= scalar;
}

template<typename T>
inline Matrix<T> operator*(const T& scalar, Matrix<T> right) {
    return right *= scalar;
}

#endif //LINEAR_ALGEBRA_LIBRARY_MATRIX_H
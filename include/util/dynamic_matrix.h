#pragma once

#include <cstdint>
#include <vector>
#include <stdexcept>

namespace g4d {

template <typename T>
class dmatrix {
    uint32_t _rows;
    uint32_t _cols;
    
    std::vector<T> _elements;

public:
    dmatrix(uint32_t d): _rows(d), _cols(d), _elements(d*d) {}
    dmatrix(uint32_t r, uint32_t c): _rows(r), _cols(c), _elements(r*c) {}

    T& operator()(uint32_t r, uint32_t c) {
        if(r >= _rows|| c >= _cols) throw std::out_of_range("Invalid row or column value");

        return _elements[static_cast<size_t>(r) * _cols + c];
    }
    
    const T& operator()(uint32_t r, uint32_t c) const {
        if (r >= _rows || c >= _cols) {
            throw std::out_of_range("Invalid row or column value");
        }
        return _elements[static_cast<size_t>(r) * _cols + c];
    }

    static dmatrix<T> dot(dmatrix<T>& left, dmatrix<T> right) {
        if(left._cols != right._rows) throw std::invalid_argument("Incompatible matrix dimensions");
        dmatrix<T> out{left._rows, right._cols};

        for(uint32_t r = 0; r < left._rows; r++) {
            for(uint32_t c = 0; c < right._cols; c++) {
                for(uint32_t i = 0; i < left._cols; i++) {
                    out(r, c) += left(r, i) * right(i, c);
                }
            }
        }

        return std::move(out);
    }

    uint32_t rows() { 
        return _rows;
    }

    uint32_t cols() {
        return _cols;
    }
};

}
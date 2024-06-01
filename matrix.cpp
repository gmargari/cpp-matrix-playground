#include <cstdio>
#include <cassert>
#include <ostream>
#include <iostream>

//==============================================================================
// Matrix
//==============================================================================
class Matrix {
public:
    constexpr Matrix(int nrow, int ncol);

    constexpr int get_nrow() const;
    constexpr int get_ncol() const;
    constexpr int get_flops() const;

    constexpr Matrix operator+(const Matrix& other) const;
    constexpr Matrix operator*(const Matrix& other) const;

    friend std::ostream& operator<<(std::ostream& os, const Matrix& mat);

private:
    constexpr Matrix(int nrow, int ncol, int flops);

    const int nrow;
    const int ncol;
    const int flops;
};

//==============================================================================
// calc_mat_add_flops ()
//==============================================================================
static constexpr int calc_mat_add_flops(const Matrix& A, const Matrix& B)
{
    return (A.get_nrow() * B.get_ncol());
}

//==============================================================================
// calc_mat_mult_flops ()
//==============================================================================
static constexpr int calc_mat_mult_flops(const Matrix& A, const Matrix& B)
{
    return (A.get_nrow() * A.get_ncol() * (2 * B.get_ncol() - 1));
}

//==============================================================================
// Matrix ()
//==============================================================================
constexpr Matrix::Matrix(int nrow, int ncol, int flops)
    : nrow(nrow),
      ncol(ncol),
      flops(flops)
{
}

//==============================================================================
// Matrix ()
//==============================================================================
constexpr Matrix::Matrix(int nrow, int ncol)
    : Matrix(nrow, ncol, 0)
{
}

//==============================================================================
// get_nrow ()
//==============================================================================
constexpr int Matrix::get_nrow() const
{
    return nrow;
}

//==============================================================================
// get_ncol ()
//==============================================================================
constexpr int Matrix::get_ncol() const
{
    return ncol;
}

//==============================================================================
// get_flops ()
//==============================================================================
constexpr int Matrix::get_flops() const
{
    return flops;
}

//==============================================================================
// operator+ ()
//==============================================================================
constexpr Matrix Matrix::operator+(const Matrix& other) const
{
    // static_assert
    if (nrow != other.nrow || this->ncol != other.ncol) {
        throw std::logic_error(("dimensions do not match: this->ncol (" + std::to_string(this->ncol) + ") != other.nrow (" + std::to_string(other.nrow) + ")").c_str());
    }

    return {
        this->nrow,
        this->ncol,
        this->flops + other.flops + calc_mat_add_flops(*this, other)
    };
}

//==============================================================================
// operator* ()
//==============================================================================
constexpr Matrix Matrix::operator*(const Matrix& other) const
{
    // static_assert
    if (this->ncol != other.nrow) {
        throw std::logic_error(("dimensions do not match: this->ncol (" + std::to_string(this->ncol) + ") != other.nrow (" + std::to_string(other.nrow) + ")").c_str());
    }

    return {
        this->nrow,
        other.ncol,
        this->flops + other.flops + calc_mat_mult_flops(*this, other)
    };
}

//==============================================================================
// operator<< ()
//==============================================================================
std::ostream& operator<<(std::ostream& os, const Matrix& mat)
{
    os << "<dims: " << mat.get_nrow() << " x " << mat.get_ncol() << ", flops: " << mat.get_flops() << ">";

    return os;
}

//==============================================================================
// compile_time_checks ()
//==============================================================================
constexpr void compile_time_checks()
{
    {
        constexpr Matrix A(2, 10);

        static_assert(A.get_nrow() == 2);
        static_assert(A.get_ncol() == 10);
        static_assert(A.get_flops() == 0);
    }

    {
        constexpr Matrix A(2, 10);
        constexpr Matrix B(2, 10);
        constexpr Matrix C = A + B;

        static_assert(C.get_nrow() == 2);
        static_assert(C.get_ncol() == 10);
        static_assert(C.get_flops() == 20);
    }

    {
        constexpr Matrix A(2, 5);
        constexpr Matrix B(5, 10);
        constexpr Matrix C = A * B;

        static_assert(C.get_nrow() == 2);
        static_assert(C.get_ncol() == 10);
        static_assert(C.get_flops() == 190);
    }

    {
        constexpr Matrix A(2, 5);
        constexpr Matrix B(5, 10);
        constexpr Matrix C(10, 3);
        constexpr Matrix D(3, 8);
        constexpr Matrix E(2, 7);
        constexpr Matrix F(7, 8);
        constexpr Matrix G = A * B * C * D + E * F;
        constexpr Matrix G2 = (A * (B * C)) * D + E * F;

        static_assert(G.get_nrow() == 2);
        static_assert(G.get_ncol() == 8);
        static_assert(G2.get_nrow() == 2);
        static_assert(G2.get_ncol() == 8);
    }
}

//==============================================================================
// main ()
//==============================================================================
int main()
{
    Matrix A(2, 5);
    Matrix B(5, 3);
    Matrix C(3, 10);

    std::cout << "(A * B) * C: " << (A * B) * C << '\n';
    std::cout << "A * (B * C): " << A * (B * C) << '\n';
}
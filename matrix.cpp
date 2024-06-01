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
    constexpr Matrix& operator+=(const Matrix& other);
    constexpr Matrix& operator*=(const Matrix& other);

    friend std::ostream& operator<<(std::ostream& os, const Matrix& mat);

private:
    constexpr Matrix(int nrow, int ncol, int flops);

    int m_nrow;
    int m_ncol;
    int m_flops;
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
    : m_nrow(nrow),
      m_ncol(ncol),
      m_flops(flops)
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
    return m_nrow;
}

//==============================================================================
// get_ncol ()
//==============================================================================
constexpr int Matrix::get_ncol() const
{
    return m_ncol;
}

//==============================================================================
// get_flops ()
//==============================================================================
constexpr int Matrix::get_flops() const
{
    return m_flops;
}

std::string diff_dims_error(const Matrix& A, const Matrix& B)
{
    return std::string("A: ") + std::to_string(A.get_nrow()) + "x" + std::to_string(A.get_ncol()) +
          ", B: " + std::to_string(B.get_nrow()) + "x" + std::to_string(B.get_ncol());
}

//==============================================================================
// operator+ ()
//==============================================================================
constexpr Matrix Matrix::operator+(const Matrix& other) const
{
    // static_assert
    if (m_nrow != other.m_nrow || this->m_ncol != other.m_ncol) {
        throw std::logic_error(("add: dimensions do not match: " + diff_dims_error(*this, other)).c_str());
    }

    return {
        this->m_nrow,
        this->m_ncol,
        this->m_flops + other.m_flops + calc_mat_add_flops(*this, other)
    };
}

//==============================================================================
// operator* ()
//==============================================================================
constexpr Matrix Matrix::operator*(const Matrix& other) const
{
    // static_assert
    if (this->m_ncol != other.m_nrow) {
        throw std::logic_error(("mult: dimensions do not match: " + diff_dims_error(*this, other)).c_str());
    }

    return {
        this->m_nrow,
        other.m_ncol,
        this->m_flops + other.m_flops + calc_mat_mult_flops(*this, other)
    };
}

//==============================================================================
// operator+= ()
//==============================================================================
constexpr Matrix& Matrix::operator+=(const Matrix& other)
{
    *this = this->operator+(other);

    return *this;
}

//==============================================================================
// operator*= ()
//==============================================================================
constexpr Matrix& Matrix::operator*=(const Matrix& other)
{
    *this = this->operator*(other);

    return *this;
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
// run_time_checks ()
//==============================================================================
void run_time_checks()
{
    {
        Matrix A(2, 10);
        Matrix B(2, 10);
        A += B;

        assert(A.get_nrow() == 2);
        assert(A.get_ncol() == 10);
        assert(A.get_flops() == 20);
    }

    {
        Matrix A(2, 10);
        Matrix B(2, 10);
        Matrix C(2, 10);
        C += A + B;

        assert(C.get_nrow() == 2);
        assert(C.get_ncol() == 10);
        assert(C.get_flops() == 40);
    }

    {
        Matrix A(2, 5);
        Matrix B(5, 10);
        A *= B;

        assert(A.get_nrow() == 2);
        assert(A.get_ncol() == 10);
        assert(A.get_flops() == 190);
    }

    {
        Matrix A(2, 5);
        Matrix B(5, 10);
        Matrix C(10, 2);
        C *= A * B;

        assert(C.get_nrow() == 10);
        assert(C.get_ncol() == 10);
        assert(C.get_flops() == 570);
    }
}

//==============================================================================
// main ()
//==============================================================================
int main()
{
    compile_time_checks();
    run_time_checks();

    Matrix A(2, 5);
    Matrix B(5, 3);
    Matrix C(3, 10);

    std::cout << "(A * B) * C: " << (A * B) * C << '\n';
    std::cout << "A * (B * C): " << A * (B * C) << '\n';
}
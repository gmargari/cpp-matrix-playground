#include <cstdio>
#include <cassert>
#include <ostream>
#include <iostream>
#include <vector>
#include <limits>
#include <string>
using namespace std::literals::string_literals;

//==============================================================================
// for_each_adjacent_pair ()
//==============================================================================
template <typename Iter, typename Func>
constexpr void for_each_adjacent_pair(Iter first, Iter last, Func func)
{
    if (first == last || std::next(first) == last) {
        return;  // Not enough elements for pairs
    }

    auto it = first;
    while (std::next(it) != last) {
        func(*it, *std::next(it));
        ++it;
    }
}

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

//==============================================================================
// diff_dims_error ()
//==============================================================================
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
// calc_mult_flops ()
//==============================================================================
constexpr int calc_mult_flops(std::initializer_list<const Matrix> mats)
{
    if (mats.size() == 0) {
        return 0;
    }

    Matrix res = *(mats.begin());
    for (auto it = mats.begin() + 1; it != mats.end(); it++) {
        res *= *it;
    }

    return res.get_flops();
}

//==============================================================================
// order_to_string ()
//==============================================================================
std::string order_to_string(std::vector<std::vector<int>>& min_index, int i, int j,
                            std::initializer_list<const char *>& mat_names)
{
    if (i == j) {
        return *(mat_names.begin() + i);
    } else {
        return "(" + order_to_string(min_index, i, min_index[i][j], mat_names) +
        " x " + order_to_string(min_index, min_index[i][j] + 1, j, mat_names) + ")";
    }
}

//==============================================================================
// calc_optimal_mult_order ()
//==============================================================================
std::pair<std::string, int> calc_optimal_mult_order(std::initializer_list<const Matrix> mats,
                                                    std::initializer_list<const char *> mat_names)
{
    //--------------------------------------------------------------------------
    // Matrix Chain Multiplication
    //--------------------------------------------------------------------------

    if (mats.size() == 0) {
        return {"", 0};
    }

    // assert mat dimensions are compatible
    for_each_adjacent_pair(mats.begin(), mats.end(),
        [](const Matrix& m1, const Matrix& m2) {
            assert(m1.get_ncol() == m2.get_nrow());
        }
    );

    const auto get_mat = [&mats](int i) { return *(mats.begin() + i); };
    const std::size_t n = mats.size();
    std::vector<std::vector<int>> min_cost(n, std::vector<int>(n));
    std::vector<std::vector<int>> min_index(n, std::vector<int>(n));
    for (std::size_t length = 2; length < n + 1; length++) {
        // find minimum flops for all chains of size 'length'
        for (std::size_t i = 0; i < n - length + 1; i++) {
            const std::size_t j = i + length - 1;

            min_cost[i][j] = std::numeric_limits<int>::max();
            for (std::size_t k = i; k < j; k++) {
                const int cost = min_cost[i][k] + min_cost[k + 1][j] +
                    // flops(dim1, dim2, dim3) = dim1 * dim2 * (2 * dim3 - 1)
                    get_mat(i).get_nrow() * get_mat(k).get_ncol() * (2 * get_mat(j).get_ncol() - 1);
                if (cost < min_cost[i][j]) {
                    min_cost[i][j] = cost;
                    min_index[i][j] = k;
                }
            }
        }
    }

    const std::string& opt_order = order_to_string(min_index, 0, n - 1, mat_names);
    const int opt_flops = min_cost[0][n - 1];

    return {opt_order, opt_flops};
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

    {
        constexpr Matrix A(2, 5);
        constexpr Matrix B(5, 10);
        constexpr Matrix C(10, 3);
        constexpr Matrix D(3, 8);

        static_assert((A * B * C * D).get_flops() == calc_mult_flops({A, B, C, D}));
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

    {
        constexpr Matrix A(40, 20);
        constexpr Matrix B(20, 30);
        constexpr Matrix C(30, 10);
        constexpr Matrix D(10, 30);

        assert(calc_optimal_mult_order({A, B, C, D}, {"A", "B", "C", "D"}) ==
               std::make_pair("((A x (B x C)) x D)"s, 50200));
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
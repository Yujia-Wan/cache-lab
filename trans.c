/**
 * @file trans.c
 * @brief Contains various implementations of matrix transpose
 *
 * Each transpose function must have a prototype of the form:
 *   void trans(size_t M, size_t N, double A[N][M], double B[M][N],
 *              double tmp[TMPCOUNT]);
 *
 * All transpose functions take the following arguments:
 *
 *   @param[in]     M    Width of A, height of B
 *   @param[in]     N    Height of A, width of B
 *   @param[in]     A    Source matrix
 *   @param[out]    B    Destination matrix
 *   @param[in,out] tmp  Array that can store temporary double values
 *
 * A transpose function is evaluated by counting the number of hits and misses,
 * using the cache parameters and score computations described in the writeup.
 *
 * Programming restrictions:
 *   - No out-of-bounds references are allowed
 *   - No alterations may be made to the source array A
 *   - Data in tmp can be read or written
 *   - This file cannot contain any local or global doubles or arrays of doubles
 *   - You may not use unions, casting, global variables, or
 *     other tricks to hide array data in other forms of local or global memory.
 *
 * @author Yujia Wang <yujiawan@andrew.cmu.edu>
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "cachelab.h"

/**
 * @brief Checks if B is the transpose of A.
 *
 * You can call this function inside of an assertion, if you'd like to verify
 * the correctness of a transpose function.
 *
 * @param[in]     M    Width of A, height of B
 * @param[in]     N    Height of A, width of B
 * @param[in]     A    Source matrix
 * @param[out]    B    Destination matrix
 *
 * @return True if B is the transpose of A, and false otherwise.
 */
#ifndef NDEBUG
static bool is_transpose(size_t M, size_t N, double A[N][M], double B[M][N]) {
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                fprintf(stderr,
                        "Transpose incorrect.  Fails for B[%zd][%zd] = %.3f, "
                        "A[%zd][%zd] = %.3f\n",
                        j, i, B[j][i], i, j, A[i][j]);
                return false;
            }
        }
    }
    return true;
}
#endif

/*
 * You can define additional transpose functions here. We've defined
 * some simple ones below to help you get started, which you should
 * feel free to modify or delete.
 */
static void trans_blocking(size_t M, size_t N, double A[N][M], double B[M][N],
                           double tmp[TMPCOUNT]) {
    assert(M > 0);
    assert(N > 0);

    for (size_t i = 0; i < N; i += 8) {
        for (size_t j = 0; j < M; j += 8) {
            for (size_t m = i; m < i + 8; m++) {
                for (size_t n = j; n < j + 8; n++) {
                    B[n][m] = A[m][n];
                }
            }
        }
    }

    assert(is_transpose(M, N, A, B));
}

static void trans_blocking_diagonal(size_t M, size_t N, double A[N][M],
                                    double B[M][N], double tmp[TMPCOUNT]) {
    assert(M > 0);
    assert(N > 0);

    for (size_t i = 0; i < N; i += 8) {
        for (size_t j = 0; j < M; j += 8) {
            for (size_t m = i; m < i + 8; m++) {
                if (i == j) {
                    tmp[0] = A[m][j];
                    tmp[1] = A[m][j + 1];
                    tmp[2] = A[m][j + 2];
                    tmp[3] = A[m][j + 3];
                    tmp[4] = A[m][j + 4];
                    tmp[5] = A[m][j + 5];
                    tmp[6] = A[m][j + 6];
                    tmp[7] = A[m][j + 7];
                    B[j][m] = tmp[0];
                    B[j + 1][m] = tmp[1];
                    B[j + 2][m] = tmp[2];
                    B[j + 3][m] = tmp[3];
                    B[j + 4][m] = tmp[4];
                    B[j + 5][m] = tmp[5];
                    B[j + 6][m] = tmp[6];
                    B[j + 7][m] = tmp[7];
                } else {
                    for (size_t n = j; n < j + 8; n++) {
                        B[n][m] = A[m][n];
                    }
                }
            }
        }
    }

    assert(is_transpose(M, N, A, B));
}

/**
 * @brief A simple baseline transpose function, not optimized for the cache.
 *
 * Note the use of asserts (defined in assert.h) that add checking code.
 * These asserts are disabled when measuring cycle counts (i.e. when running
 * the ./test-trans) to avoid affecting performance.
 */
static void trans_basic(size_t M, size_t N, double A[N][M], double B[M][N],
                        double tmp[TMPCOUNT]) {
    assert(M > 0);
    assert(N > 0);

    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; j++) {
            B[j][i] = A[i][j];
        }
    }

    assert(is_transpose(M, N, A, B));
}

/**
 * @brief A contrived example to illustrate the use of the temporary array.
 *
 * This function uses the first four elements of tmp as a 2x2 array with
 * row-major ordering.
 */
static void trans_tmp(size_t M, size_t N, double A[N][M], double B[M][N],
                      double tmp[TMPCOUNT]) {
    assert(M > 0);
    assert(N > 0);

    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; j++) {
            size_t di = i % 2;
            size_t dj = j % 2;
            tmp[2 * di + dj] = A[i][j];
            B[j][i] = tmp[2 * di + dj];
        }
    }

    assert(is_transpose(M, N, A, B));
}

/**
 * @brief The solution transpose function that will be graded.
 *
 * You can call other transpose functions from here as you please.
 * It's OK to choose different functions based on array size, but
 * this function must be correct for all values of M and N.
 */
static void transpose_submit(size_t M, size_t N, double A[N][M], double B[M][N],
                             double tmp[TMPCOUNT]) {
    if (M == N) {
        if (M == 32) {
            trans_blocking_diagonal(M, N, A, B, tmp);
        } else if (M == 1024) {
            trans_blocking(M, N, A, B, tmp);
        } else {
            trans_basic(M, N, A, B, tmp);
        }
    } else {
        trans_tmp(M, N, A, B, tmp);
    }
}

/**
 * @brief Registers all transpose functions with the driver.
 *
 * At runtime, the driver will evaluate each function registered here, and
 * and summarize the performance of each. This is a handy way to experiment
 * with different transpose strategies.
 */
void registerFunctions(void) {
    // Register the solution function. Do not modify this line!
    registerTransFunction(transpose_submit, SUBMIT_DESCRIPTION);

    // Register any additional transpose functions
    registerTransFunction(trans_blocking, "Transpose using blocking");
    registerTransFunction(trans_blocking_diagonal,
                          "Transpose using blocking and diagonal");
}

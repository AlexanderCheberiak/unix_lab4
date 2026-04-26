#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>

using namespace std;
using namespace std::chrono;

// Функція для множення частини рядків матриці
void multiply_chunk(const vector<vector<int>>& A, const vector<vector<int>>& B, vector<vector<int>>& C, 
                    int start_row, int end_row, int m, int k) {
    for (int r = start_row; r < end_row; ++r) {
        for (int c = 0; c < k; ++c) {
            int sum = 0;
            for (int x = 0; x < m; ++x) {
                sum += A[r][x] * B[x][c];
            }
            C[r][c] = sum;
        }
    }
}

// Функція-обгортка для управління потоками та вимірювання часу
void test_parallel_multiplication(int n, int m, int k, int num_threads, const vector<vector<int>>& A, const vector<vector<int>>& B) {
    vector<vector<int>> C(n, vector<int>(k, 0));
    vector<thread> threads;
    
    // Розрахунок кількості рядків на один потік
    int rows_per_thread = n / num_threads;
    int remainder = n % num_threads;

    auto start_time = high_resolution_clock::now();

    int current_row = 0;
    for (int i = 0; i < num_threads; ++i) {
        int rows_for_this_thread = rows_per_thread + (i < remainder ? 1 : 0);
        int end_row = current_row + rows_for_this_thread;
        
        if (rows_for_this_thread > 0) {
            threads.push_back(thread(multiply_chunk, cref(A), cref(B), ref(C), current_row, end_row, m, k));
        }
        current_row = end_row;
    }

    for (auto& th : threads) {
        th.join();
    }

    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);

    cout << "Потоків: " << num_threads << "\t Час: " << duration.count() << " мс" << endl;
}

int main() {
    // Розмір матриць: 1000x1000 (досить великі для відчутного часу обчислень)
    int size = 1000;
    int n = size, m = size, k = size;

    cout << "Генерація матриць " << size << "x" << size << "..." << endl;
    vector<vector<int>> A(n, vector<int>(m, 1)); // Для швидкості генерації заповнюємо 1
    vector<vector<int>> B(m, vector<int>(k, 1));

    int hw_threads = thread::hardware_concurrency();
    cout << "Апаратних потоків доступно: " << hw_threads << "\n" << endl;

    vector<int> thread_counts = {1, 2, 4, 8, hw_threads, hw_threads * 2, 32, 64};

    for (int t : thread_counts) {
        test_parallel_multiplication(n, m, k, t, A, B);
    }

    return 0;
}
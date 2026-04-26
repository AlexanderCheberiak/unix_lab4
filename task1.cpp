#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>

using namespace std;
using namespace std::chrono;

mutex cout_mutex;

// 1.1: 1 потік на 1 клітинку
void compute_cell_and_print(const vector<vector<int>>& A, const vector<vector<int>>& B, vector<vector<int>>& C, int r, int c, int m) {
    int sum = 0;
    for (int i = 0; i < m; ++i) {
        sum += A[r][i] * B[i][c];
    }
    C[r][c] = sum;

    this_thread::sleep_for(chrono::milliseconds(rand() % 50));

    lock_guard<mutex> lock(cout_mutex);
    cout << "[" << r << "," << c << "]=" << sum << endl;
}

// ЗАВДАННЯ 1.2*: Дослідження швидкодії (Розподіл рядків між потоками)
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

void test_performance(int n, int m, int k, int num_threads, const vector<vector<int>>& A, const vector<vector<int>>& B) {
    vector<vector<int>> C(n, vector<int>(k, 0));
    vector<thread> threads;
    
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

    cout << "Потокiв: " << num_threads << "\t Час: " << duration.count() << " мс" << endl;
}

int main() {
    srand(time(nullptr));

    // 1: Демонстрація паралелізму
    cout << "Демонстрацiя непослiдовностi (1.1)" << endl;
    int n1 = 3, m1 = 3, k1 = 3;
    vector<vector<int>> A1(n1, vector<int>(m1, 2));
    vector<vector<int>> B1(m1, vector<int>(k1, 3));
    vector<vector<int>> C1(n1, vector<int>(k1, 0));

    cout << "Запуск " << (n1 * k1) << " потокiв для матрицi " << n1 << "x" << k1 << "..." << endl;
    vector<thread> demo_threads;
    for (int i = 0; i < n1; ++i) {
        for (int j = 0; j < k1; ++j) {
            demo_threads.push_back(thread(compute_cell_and_print, cref(A1), cref(B1), ref(C1), i, j, m1));
        }
    }
    for (auto& th : demo_threads) {
        th.join();
    }

    // 2: Дослідження швидкодії
    cout << "\n Дослiдження швидкодiї (1.2*) " << endl;
    int size = 500; 
    cout << "Генерацiя матриць " << size << "x" << size << "..." << endl;
    vector<vector<int>> A2(size, vector<int>(size, 1));
    vector<vector<int>> B2(size, vector<int>(size, 1));

    int hw_threads = thread::hardware_concurrency();
    cout << "Всього потокiв доступно: " << hw_threads << "\n" << endl;

    // Тестуємо різну кількість потоків
    vector<int> thread_counts = {1, 2, 4, hw_threads, hw_threads * 2, 64, 128};

    for (int t : thread_counts) {
        test_performance(size, size, size, t, A2, B2);
    }

    return 0;
}
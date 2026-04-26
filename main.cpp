#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <random>
#include <chrono>

using namespace std;

// М'ютекс для коректного виведення в консоль із різних потоків
mutex cout_mutex;

// ==========================================
// ЗАВДАННЯ 1: Функція для множення матриць
// ==========================================
void compute_cell(const vector<vector<int>>& A, const vector<vector<int>>& B, vector<vector<int>>& C, int r, int c, int m) {
    int sum = 0;
    for (int i = 0; i < m; ++i) {
        sum += A[r][i] * B[i][c];
    }
    C[r][c] = sum;

    // Імітація різного часу виконання для демонстрації непослідовності
    this_thread::sleep_for(chrono::milliseconds(rand() % 50));

    // Блокуємо консоль для чистого виведення
    lock_guard<mutex> lock(cout_mutex);
    cout << "[" << r << "," << c << "]=" << sum << endl;
}

// ==========================================
// ЗАВДАННЯ 2: Робота зі спільною пам'яттю
// ==========================================
long long v_unsafe = 0; // Без захисту
long long v_safe = 0;   // Із захистом
mutex v_mutex;          // М'ютекс для критичної секції

// Функція БЕЗ критичної секції
void increment_unsafe(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        v_unsafe = v_unsafe + 1; 
    }
}

// Функція З критичною секцією (м'ютекс)
void increment_safe(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        lock_guard<mutex> lock(v_mutex); // Вхід у критичну секцію
        v_safe = v_safe + 1;
        // Вихід з критичної секції (деструктор lock_guard)
    }
}

int main() {
    srand(time(nullptr));

    // ---------------------------------------------------------
    // Виконання Завдання 1
    // ---------------------------------------------------------
    cout << "--- ЗАВДАННЯ 1: Множення матриць ---" << endl;
    int n = 3, m = 2, k = 3;
    
    // Ініціалізація матриць випадковими числами (від 1 до 5)
    vector<vector<int>> A(n, vector<int>(m));
    vector<vector<int>> B(m, vector<int>(k));
    vector<vector<int>> C(n, vector<int>(k, 0));

    for(int i=0; i<n; ++i) for(int j=0; j<m; ++j) A[i][j] = rand() % 5 + 1;
    for(int i=0; i<m; ++i) for(int j=0; j<k; ++j) B[i][j] = rand() % 5 + 1;

    vector<thread> matrix_threads;

    // Створення потоків: 1 потік на кожен елемент результуючої матриці
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < k; ++j) {
            matrix_threads.push_back(thread(compute_cell, cref(A), cref(B), ref(C), i, j, m));
        }
    }

    // Очікування завершення всіх потоків матриці
    for (auto& th : matrix_threads) {
        th.join();
    }


    // ---------------------------------------------------------
    // Виконання Завдання 2
    // ---------------------------------------------------------
    cout << "\n--- ЗАВДАННЯ 2: Спільна комірка пам'яті ---" << endl;
    
    // Примітка: 10^9 ітерацій з м'ютексом може виконуватись дуже довго (кілька хвилин).
    // Для комфортної демонстрації використано 10^7 (10 мільйонів). 
    // Ви можете змінити на 1000000000 за бажанням.
    int iterations = 10000000; 

    cout << "Запуск двох потоків БЕЗ критичної секції (по " << iterations << " ітерацій)..." << endl;
    thread t1_unsafe(increment_unsafe, iterations);
    thread t2_unsafe(increment_unsafe, iterations);
    t1_unsafe.join();
    t2_unsafe.join();

    cout << "Запуск двох потоків З критичною секцією (по " << iterations << " ітерацій)..." << endl;
    thread t1_safe(increment_safe, iterations);
    thread t2_safe(increment_safe, iterations);
    t1_safe.join();
    t2_safe.join();

    // Виведення результатів
    cout << "\nОчікуваний результат: " << (iterations * 2) << endl;
    cout << "Результат БЕЗ критичного сегменту (v_unsafe): " << v_unsafe << " (втрата даних!)" << endl;
    cout << "Результат З критичним сегментом (v_safe): " << v_safe << " (дані збережено)" << endl;

    return 0;
}
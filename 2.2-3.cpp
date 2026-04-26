#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

using namespace std;
using namespace std::chrono;

const int ITERATIONS = 100000000; // 100 мільйонів!

// =========================================
// ВАРІАНТ 1: Використання std::atomic
// =========================================
atomic<long long> v_atomic{0};

void increment_atomic() {
    for (int i = 0; i < ITERATIONS; ++i) {
        v_atomic++; // Апаратна атомарна операція (без м'ютекса)
    }
}

// =========================================
// ВАРІАНТ 2: Локальні акумулятори (Найкращий)
// =========================================
long long v_fastest = 0;
mutex final_mutex; // Використовується лише в кінці

void increment_local_accumulator() {
    long long local_sum = 0; // Локальна змінна, до неї має доступ лише цей потік

    // Жодної синхронізації у важкому циклі! Працює на максимальній швидкості процесора.
    for (int i = 0; i < ITERATIONS; ++i) {
        local_sum++; 
    }

    // Синхронізуємось лише ОДИН раз у кінці, щоб додати локальний результат до глобального
    lock_guard<mutex> lock(final_mutex);
    v_fastest += local_sum;
}

int main() {
    cout << "Кількість ітерацій на потік: " << ITERATIONS << endl;

    // --- Тестування std::atomic ---
    auto start = high_resolution_clock::now();
    thread t1(increment_atomic);
    thread t2(increment_atomic);
    t1.join(); t2.join();
    auto end = high_resolution_clock::now();
    
    cout << "\n[std::atomic] Результат: " << v_atomic << " (Коректно!)" << endl;
    cout << "[std::atomic] Час виконання: " << duration_cast<milliseconds>(end - start).count() << " мс" << endl;

    // --- Тестування локальних акумуляторів ---
    start = high_resolution_clock::now();
    thread t3(increment_local_accumulator);
    thread t4(increment_local_accumulator);
    t3.join(); t4.join();
    end = high_resolution_clock::now();
    
    cout << "\n[Локальний акумулятор] Результат: " << v_fastest << " (Коректно!)" << endl;
    cout << "[Локальний акумулятор] Час виконання: " << duration_cast<milliseconds>(end - start).count() << " мс" << endl;

    return 0;
}
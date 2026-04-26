#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <barrier>
#include <vector>

using namespace std;
using namespace std::chrono;

// Кількість ітерацій для завдань 2.1, 2.2, 2.3
const int ITERS_LARGE = 100000000; 

// Кількість ітерацій для завдання 2.3* 
const int ITERS_SYNC = 1000;

// Глобальні змінні для різних варіантів
long long v_unsafe = 0;              // 2.б: Без критичної секції
long long v_mutex = 0;               // 2.а: З використанням mutex
long long v_fastest = 0;             // 2.3: Найшвидший метод (локальні змінні)
int v_sync = 0;                      // 2.3*: Строга синхронізація

mutex mtx;                           // М'ютекс для 2.а
mutex mtx_fastest;                   // М'ютекс для  додавання в 2.3
barrier sync_point(2);               // Бар'єр для 2 потоків для 2.3*

// Функції-обробники для потоків

// Варіант 2.б: Без критичного сегменту
void inc_unsafe() {
    for (int i = 0; i < ITERS_LARGE; ++i) {
        v_unsafe = v_unsafe + 1;
    }
}

// Варіант 2.а: З використанням mutex
void inc_mutex() {
    for (int i = 0; i < ITERS_LARGE; ++i) {
        lock_guard<mutex> lock(mtx);
        v_mutex = v_mutex + 1;
    }
}

// Варіант 2.3: Найшвидший коректний результат
void inc_fastest() {
    long long local_sum = 0;
    // Цикл працює без блокувань
    for (int i = 0; i < ITERS_LARGE; ++i) {
        local_sum++;
    }
    // Єдине блокування в кінці
    lock_guard<mutex> lock(mtx_fastest);
    v_fastest += local_sum;
}

// Варіант 2.3*: Строга синхронізація крок-в-крок
void inc_sync() {
    for (int i = 0; i < ITERS_SYNC; ++i) {
        sync_point.arrive_and_wait(); // Чекаємо один одного перед читанням
        int local_v = v_sync;         // Обидва читають однакове значення
        
        sync_point.arrive_and_wait(); // Чекаємо завершення читання обома
        v_sync = local_v + 1;         // Обидва записують однаковий результат
        
        sync_point.arrive_and_wait(); // Чекаємо завершення запису
    }
}

int main() {
    cout << "Кiлькiсть iтерацiй для навантажувальних тестiв: " << ITERS_LARGE << endl;
    cout << "Очiкуваний результат без втрат: " << (ITERS_LARGE * 2LL) << "\n\n";

    //  2.б:Без критичної секції
    auto start = high_resolution_clock::now();
    thread t1(inc_unsafe), t2(inc_unsafe);
    t1.join(); t2.join();
    auto end = high_resolution_clock::now();
    cout << "2.б " << endl;
    cout << "Результат: " << v_unsafe << " (Втрата: " << (ITERS_LARGE * 2LL) - v_unsafe << ")" << endl;
    cout << "Час: " << duration_cast<milliseconds>(end - start).count() << " мс\n\n";

    //  2.а: З мutex
    start = high_resolution_clock::now();
    thread t3(inc_mutex), t4(inc_mutex);
    t3.join(); t4.join();
    end = high_resolution_clock::now();
    cout << "2.а " << endl;
    cout << "Результат: " << v_mutex  << endl;
    cout << "Час: " << duration_cast<milliseconds>(end - start).count() << " мс\n\n";

    //  2.3: Найшвидший коректний метод
    start = high_resolution_clock::now();
    thread t5(inc_fastest), t6(inc_fastest);
    t5.join(); t6.join();
    end = high_resolution_clock::now();
    cout << "2.3 " << endl;
    cout << "Результат: " << v_fastest  << endl;
    cout << "Час: " << duration_cast<milliseconds>(end - start).count() << " мс\n\n";

    // 2.3* 
    cout << "2.3*" << endl;
    cout << "Кiлькiсть iтерацiй на потiк: " << ITERS_SYNC << endl;
    start = high_resolution_clock::now();
    thread t7(inc_sync), t8(inc_sync);
    t7.join(); t8.join();
    end = high_resolution_clock::now();
    cout << "Очiкувалось: " << (ITERS_SYNC * 2) << endl;
    cout << "Реальний результат: " << v_sync  << endl;

    return 0;
}
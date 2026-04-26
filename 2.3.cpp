#include <iostream>
#include <thread>
#include <barrier>

using namespace std;

int v = 0;
const int ITERATIONS = 1000;

// Створюємо бар'єр, який чекатиме на 2 потоки
barrier sync_point(2);

void lockstep_increment() {
    for (int i = 0; i < ITERATIONS; ++i) {
        // 1. Синхронізація ПЕРЕД зчитуванням. 
        // Чекаємо, поки обидва потоки будуть готові почати ітерацію одночасно.
        sync_point.arrive_and_wait();
        
        // 2. Обидва потоки зчитують однакове поточне значення v у локальну змінну
        int local_v = v; 
        
        // 3. Синхронізація ПІСЛЯ зчитування.
        // Гарантуємо, що жоден з потоків не перейде до етапу запису, 
        // поки інший потік не завершить своє читання.
        sync_point.arrive_and_wait();
        
        // 4. Обидва потоки обчислюють local_v + 1 і записують його у спільну v.
        // Оскільки local_v у них абсолютно однаковий, вони двічі запишуть 
        // одне й те саме число (наприклад, обидва запишуть "1", потім обидва "2").
        v = local_v + 1;
        
        // 5. Синхронізація ПІСЛЯ запису.
        // Жоден потік не почне нову ітерацію, поки обидва не завершать запис.
        sync_point.arrive_and_wait();
    }
}

int main() {
    cout << "Запуск строго синхронного виконання (lock-step)..." << endl;
    
    thread t1(lockstep_increment);
    thread t2(lockstep_increment);
    
    t1.join();
    t2.join();
    
    cout << "Кількість ітерацій на кожен потік: " << ITERATIONS << endl;
    cout << "Очікуваний результат без колізій: " << (ITERATIONS * 2) << endl;
    cout << "Реальний результат (повне перекриття): " << v << endl;
    
    return 0;
}
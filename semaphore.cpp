#include <iostream>
#include <deque>
#include <thread>
#include <fstream>
#include <atomic>
#include <mutex>
#include <semaphore.h>
using namespace std;

const int BUFFER_SIZE = 100;
deque<int> buffer; // shared buffer
atomic<int> count(0); // shared counter
sem_t lockSem; // semaphore for lock
sem_t slotsSem; // semaphore for available slots in buffer
mutex cout_mutex; // mutex for cout

void updateLog(bool isPro) {
    ofstream file("log.txt", ios::app);
    if (file.fail())
        cout << "can not open a file\n";
	if(isPro)
		file << "Item is pushed in buffer.\n";
	else
		file << "Item is popped from buffer.\n";
    file << "buffer state: ";
    for (const auto &item : buffer)
        file << item << " ";
    file << "\ncount = " << count << "\n\n";
}

void producer(int newProduct, int p_id) {
    int nextProduced = newProduct;
    sem_wait(&slotsSem); // Wait for available slot
    {
        lock_guard<mutex> cout_lock(cout_mutex); // Lock cout
        cout << "process " << p_id << " trying to enter the critical section" << endl;
    }
    sem_wait(&lockSem); // Acquire lock
    buffer.push_front(nextProduced);
    sem_post(&lockSem); // Release lock
    sem_post(&slotsSem); // Increment available slot
    {
        lock_guard<mutex> cout_lock(cout_mutex); // Lock cout
        cout << "process " << p_id << " entered the critical section" << endl;
        cout << "process " << p_id << " exited the critical section" << endl;
    }
    // Remainder code
    updateLog(true);
}

void consumer(int p_id) {
    sem_wait(&slotsSem); // Wait for available slot
    {
        lock_guard<mutex> cout_lock(cout_mutex); // Lock cout
        cout << "process " << p_id << " trying to enter the critical section" << endl;
    }
    sem_wait(&lockSem); // Acquire lock
    int nextConsumed = buffer.back();
    buffer.pop_back();
    sem_post(&lockSem); // Release lock
    sem_post(&slotsSem); // Increment available slot
    {
        lock_guard<mutex> cout_lock(cout_mutex); // Lock cout
        cout << "process " << p_id << " entered the critical section" << endl;
        cout << "process " << p_id << " exited the critical section" << endl;
    }
    // Remainder code
    updateLog(false);
}

void clear(string fileName) {
    ofstream file(fileName, ios::trunc);
    if (file.fail()) {
        cout << "can not open a file!" << endl;
        return;
    }
    file.close();
}

int main() {
    sem_init(&lockSem, 0, 1); // Initialize lock semaphore
    sem_init(&slotsSem, 0, BUFFER_SIZE); // Initialize available slots semaphore

    clear("log.txt");
    /*
    expect some multithreading in the middle
    but skip it for now:)
    */
    thread t0(producer, 9, 0);
    thread t1(producer, 11, 1);
    thread t2(producer, 10, 2);
    thread t3(consumer, 3);
	thread t4(consumer, 4);
    t0.join();
    t1.join();
    t2.join();
    t3.join();
	t4.join();

    sem_destroy(&lockSem);
    sem_destroy(&slotsSem);

    return 0;
}

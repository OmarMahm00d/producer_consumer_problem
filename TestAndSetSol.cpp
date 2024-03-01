/*
-----general solution-----
	some code..
	
	acquire lock
	<<critical section>>
	release lock
	
	remainder code
*/

#include <iostream>
#include <deque>
#include <thread>
#include <fstream>
#include <atomic>
#include <mutex>
using namespace std;

/*
implementation using test&set "busy wait":
*/

const int BUFFER_SIZE = 100;
deque<int> buffer; // shared buffer
atomic<int> count(0); // shared counter
atomic<bool> lockFlag(false); // shared lock for busy waiting
mutex mtx; // mutex for critical section
mutex cout_mutex; // mutex for cout

void updateLog(bool isPro) {
    ofstream file("log.txt", ios::app);
    if (file.fail())
        cout << "can not open a file\n";
	if(isPro)
		file << "Item is pushed in buffer.\n";
	else
		file << "Item is poped from buffer.\n";
    file << "buffer state: ";
    for (const auto &item : buffer)
        file << item << " ";
    file << "\ncount = " << count << "\n\n";
}

void producer(int newProduct, int p_id) {
    int nextProduced = newProduct;
    while (true) {
        // Busy waiting
        while (count == BUFFER_SIZE || lockFlag.exchange(true)) {
            {
                lock_guard<mutex> cout_lock(cout_mutex); // Lock cout
                cout << "process " << p_id << " trying to enter the critical section" << endl;
            }
        }
        if (buffer.size() != BUFFER_SIZE) {
            {
                lock_guard<mutex> lock(mtx); // Lock the critical section using mutex
                buffer.push_front(nextProduced);
                {
                    lock_guard<mutex> cout_lock(cout_mutex); // Lock cout
                    cout << "process " << p_id << " entered the critical section" << endl;
                }
				cout << "process " << p_id << " is performing operation on the critical section" << endl;
                count++;
            }
            {
                lock_guard<mutex> cout_lock(cout_mutex); // Lock cout
                cout << "process " << p_id << " exited the critical section" << endl;
            }
            lockFlag = false; // Release lock
            break;
        }
        lockFlag = false; // Release lock
    }
    // Remainder code
    updateLog(1);
}

void consumer(int p_id) {
    while (true) {
        // Busy waiting
        while (count == 0 || lockFlag.exchange(true)) {
            {
                lock_guard<mutex> cout_lock(cout_mutex); // Lock cout
                cout << "process " << p_id << " trying to enter the critical section" << endl;
            }
        }
        if (count != 0) {
            {
                lock_guard<mutex> lock(mtx); // Lock the critical section using mutex
                int nextConsumed = buffer.back();
                buffer.pop_back();
                {
                    lock_guard<mutex> cout_lock(cout_mutex); // Lock cout
                    cout << "process " << p_id << " entered the critical section" << endl;
                }
				cout << "process " << p_id << " is performing operation on the critical section" << endl;
                count--;
            }
            {
                lock_guard<mutex> cout_lock(cout_mutex); // Lock cout
                cout << "process " << p_id << " exited the critical section" << endl;
            }
            lockFlag = false; // Release lock
            break;
        }
        lockFlag = false; // Release lock
    }
    // Remainder code
    updateLog(0);
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
    return 0;
}

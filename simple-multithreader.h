#include <iostream>
#include <pthread.h>
#include <functional>
#include <algorithm>
#include <vector>
#include <chrono>
using namespace std;
using namespace std::chrono;

// Struct to represent the arguments for a 2D loop
typedef struct {
    int low1, low2;
    int high1, high2;
    int start;
    int end;
    int ncol;  
    function<void(int, int)> lambda;
} TwoDforLoop;

// Function to join all threads created
static void joinThreads(int numThreads, pthread_t tid[]) {
    for (int i = 0; i < numThreads-1; i++) {
        pthread_join(tid[i+1], NULL);  // Ensure you join all threads
    }
}

// Thread function to process the 2D loop
static void* thread_func_2D(void* ptr) {
    TwoDforLoop* t = (TwoDforLoop*)ptr;
    for (int i = t->start; i < t->end; i++) {
        int row = (i / t->ncol) + t->low1;
        int col = (i % t->ncol) + t->low2;

        if (row < t->high1 && col < t->high2) {
            t->lambda(row, col);  // Call the lambda function for (row, col)
        }
    }
    return NULL;
}

// Parallelize the 2D loop
static void parallel_for(int low1, int high1, int low2, int high2, function<void(int, int)> lambda, int numThreads) {
    if (low1 >= high1 || low2 >= high2) {
        cerr << "Invalid bounds: low1 >= high1 or low2 >= high2" << endl;
        return;
    }
    auto start_time = high_resolution_clock::now();
    int total_stuff = (high1 - low1) * (high2 - low2);
    int chunk_size = (total_stuff + numThreads - 1) / numThreads;  // Split the work into chunks

    pthread_t* tid = new pthread_t[numThreads - 1];  // Create one less thread
    TwoDforLoop* args = new TwoDforLoop[numThreads];

    // Main thread handles the first chunk
    int main_start = 0;
    int main_end = min(chunk_size, total_stuff);  // Work for the main thread
    for (int i = main_start; i < main_end; i++) {
        int row = low1 + (i / (high2 - low2));
        int col = low2 + (i % (high2 - low2));
        lambda(row, col);  // Execute the lambda for main thread's chunk
    }

    // Set up arguments and create the remaining threads
    for (int i = 1; i < numThreads; i++) {
        args[i].low1 = low1;
        args[i].low2 = low2;
        args[i].high1 = high1;
        args[i].high2 = high2;
        args[i].start = i * chunk_size;
        args[i].end = min((i + 1) * chunk_size, total_stuff);
        args[i].ncol = high2 - low2;
        args[i].lambda = lambda;

        pthread_create(&tid[i], NULL, thread_func_2D, (void*)&args[i]);
    }

    // Join all worker threads
    joinThreads(numThreads,tid);

    delete[] tid;
    delete[] args;
    auto end_time = high_resolution_clock::now();  // Capture end time

    // Calculate the duration
    auto duration = duration_cast<microseconds>(end_time - start_time);

    cout << "2D Loop Execution Completed" << endl;
    cout<<"Duration of this execution is "<<duration.count()<<" Microseconds"<<endl;
}

// Struct to represent the arguments for a 1D loop
typedef struct {
    int low;
    int high;
    function<void(int)> lambda;
} OneDforLoop;

// Thread function to process the 1D loop
static void *thread_func(void *ptr) {
    OneDforLoop *t = (OneDforLoop *)ptr;
    for (int i = t->low; i <= t->high; i++) {
        t->lambda(i);
    }
    return NULL;
}

// Parallelize the 1D loop
static void parallel_for(int low, int high, function<void(int)> lambda, int numThreads) {
    if (high <= low) {
        cerr << "This cannot be processed as low " << low << " is greater than high " << high << endl;
        return;
    }
    auto start_time = high_resolution_clock::now();
    pthread_t* tid=new pthread_t[numThreads];
    OneDforLoop* args=new OneDforLoop[numThreads];
    // vector<OneDforLoop> args(numThreads);
    int size = (high - low + 1); // Inclusive range
    int chunk_size = (size + numThreads - 1) / numThreads;
    // for(int i)
    for(int i=0;i<min(chunk_size,size);i++){
        lambda(low+i);
    }

    for (int i = 1; i < numThreads; i++) {
        args[i].low = low + i * chunk_size;
        args[i].high = min(low + (i + 1) * chunk_size - 1, high);
        args[i].lambda = lambda; // Copy the lambda function
        pthread_create(&tid[i], NULL, thread_func, (void *)&args[i]);
    }

    joinThreads(numThreads,tid);

    cout << "Execution completed" << endl;
    delete[] tid;
    delete [] args;
    auto end_time = high_resolution_clock::now();  // Capture end time
    auto duration = duration_cast<microseconds>(end_time - start_time);

    cout<<"Duration of this execution is "<<duration.count()<<" Microseconds"<<endl;
}
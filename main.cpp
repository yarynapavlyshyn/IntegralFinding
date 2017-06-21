#include <iostream>
#include <fstream>
#include <chrono>
#include <atomic>
#include <math.h>
#include <sstream>
#include "thread"
#include "mutex"

using namespace std;
double result;

mutex Mutex;

// Define current time
inline chrono::high_resolution_clock::time_point get_current_time_fenced()
{
    atomic_thread_fence(std::memory_order_seq_cst);
    auto res_time = chrono::high_resolution_clock::now();
    atomic_thread_fence(memory_order_seq_cst);
    return res_time;
}

// Convert time to microseconds
template<class D>
inline long long to_us(const D& d)
{
    return chrono::duration_cast<chrono::microseconds>(d).count();
}

// My function
double f(double x, double y, int c[], int a[], int b[], int m){
    double res = 0;
    for (int i = 0; i < m; i++){
        double k = pow((x - a[i]), 2) + pow((y - b[i]), 2);
        res -= c[i] * exp(-1/M_PI * k * cos (M_PI * k));
    }
    return res;
}
// fromX, fromY, intervals_per_thread, dx, dy, c, a, b, m
void integrate(double x_from, double y_from, int num_of_intervals, double dx, double dy, int* c, int* a, int* b, int m){
    double sumSquares = 0;
    double ix = x_from;
    for (int i = 0; i < num_of_intervals; i++){
        double iy = y_from;
        for(int j = 0; j < m; j++){
            double fij = f(ix, iy, c, a, b, m);
            fij = fij * dx * dy;
            sumSquares += fij;
            iy += dy;
        }
        ix += dx;
    }
    lock_guard<mutex> lg(Mutex);
    result += sumSquares;
}

int main() {
    string part_input_data[6];
    int equality_sign, m;

    ifstream myFile;
    myFile.open("ConfigurationFile.txt");

    for (int i = 0; i < 6; i++){
        myFile >> part_input_data[i];}

    // Determine a parameter m in function
    equality_sign = part_input_data[5].find("=");
    m = stoi(part_input_data[5].substr(equality_sign + 2, part_input_data[equality_sign].length()));

    string input_data[6 + m * 3];

    for (int i = 0; i < 6 + m * 3; i++) {
        myFile >> input_data[i];
    }
    myFile.close();

    for (int i = 0; i < 6 + m * 3; i++) {
        equality_sign = input_data[i].find("=");
        input_data[i] = (input_data[i].substr(equality_sign + 2, input_data[equality_sign].length()));
    }

    auto stage1_start_time = get_current_time_fenced();

    double x_from, x_to, y_from, y_to;
    int c[m], a[m], b[m];
    int threads, intervals;
    string* data = input_data;
    for (int i = 0; i < data->length(); i++) {
        printf(data[i].c_str());
    }
    x_from = stod(data[0]);
    x_to = stod(data[1]);
    y_from = stod(data[2]);
    y_to = stod(data[3]);
    threads = stoi(data[4]);
    // we made numbers of intervals to be in ten times larger than number of threads in ConfFile
    intervals = stoi(data[5]);
    m = stoi(data[6]); // Function parameter
    int i = 6;
    // Function parameters
    for (int k = 0; k < m; k++) {
        c[k] = stoi(data[i++]);
        a[k] = stoi(data[i++]);
        b[k] = stoi(data[i++]);
    }

    auto stage2_start_time = get_current_time_fenced();
    thread t[threads];

    int intervals_per_thread = int(intervals / threads);
    double dx = (x_to - x_from) / intervals;
    double dy = (y_to - y_from) / intervals;

    double moveXperThread = (x_to - x_from) / threads;
    double moveYperThread = (y_to - y_from) / threads;

    double fromX = x_from;
    double fromY = y_from;

    for (int tt = 0; tt < threads; tt++) {
        t[i] = thread(integrate, fromX, fromY, intervals, dx, dy, c, a, b, m);
        fromX += moveXperThread;
        fromY += moveYperThread;
    }

    //Join the threads with the main thread
    for (int j = 0; i < threads; ++i) {
        t[i].join();
    }

    auto finish_time = get_current_time_fenced();

    auto total_time = finish_time - stage1_start_time;
    auto stage1_time = stage2_start_time - stage1_start_time;
    auto stage2_time = finish_time - stage2_start_time;

    cout << "Total time: " << to_us(total_time) << endl;
    cout << "Stage 1 time: " << to_us(stage1_time) << endl;
    cout << "Stage 2 time: " << to_us(stage2_time) << endl;
}
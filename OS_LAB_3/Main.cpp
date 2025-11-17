#include <iostream>
#include <vector>
#include <thread>
#include "threadfunctions.h"
#include "mathfunctions.h"

void run_marker_process() {
    int array_size = get_positive_number("Enter array size: ");
    std::vector<int> array(array_size, 0);

    int thread_count = get_positive_number("Enter number of marker threads: ");
    std::vector<MarkerData> data(thread_count);
    std::vector<std::thread> threads;
    threads.reserve(thread_count);

    create_marker_threads(data, threads, array);

    start_all.store(true);
    cv_marker.notify_all();

    int active_threads = thread_count;

    while (active_threads > 0) {
        wait_for_all_markers(active_threads);
        print_array(array, "Array state:");

        int id = choose_thread_to_terminate(data);
        if (id == -1) {
            terminate_all_threads(data, threads, array, active_threads);
            break;
        }

        terminate_thread(id, data, threads, array, active_threads);

        if (active_threads > 0)
            resume_other_markers(data);
    }

    std::cout << "ALL MARKER THREADS ARE CLOSED. Main thread closing.\n";
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    run_marker_process();
    return 0;
}
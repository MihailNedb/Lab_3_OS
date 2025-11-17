#include "threadfunctions.h"
#include "mathfunctions.h"
#include <iostream>
#include <chrono>
#include <sstream>
#include <cstdlib>

std::mutex mtx;
std::condition_variable cv_main;
std::condition_variable cv_marker;
std::atomic<bool> start_all(false);
std::atomic<int> waiting_count(0);

void marker_thread(MarkerData* data) {
    {
        std::unique_lock<std::mutex> lk(mtx);
        cv_marker.wait(lk, [] { return start_all.load(); });
    }

    std::srand(data->thread_id);
    std::vector<int> marked_indices;

    while (true) {
        int random_num = std::rand();
        int len = static_cast<int>(data->array->size());
        int index = random_num % len;

        std::unique_lock<std::mutex> lk(mtx);

        if (data->terminate.load()) {
            for (int idx : marked_indices)
                (*data->array)[idx] = 0;
            break;
        }

        if ((*data->array)[index] == 0) {
            lk.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));

            lk.lock();
            if ((*data->array)[index] == 0) {
                (*data->array)[index] = data->thread_id;
                marked_indices.push_back(index);
                data->marked_count++;
            }
            lk.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        else {
            if (data->marked_count == 1)
            {
                std::cout << "Marker thread " << data->thread_id
                << ": marked " << data->marked_count
                << " element, cannot mark index " << index << std::endl;
            }
            else
            {
                std::cout << "Marker thread " << data->thread_id
                << ": marked " << data->marked_count
                << " elements, cannot mark index " << index << std::endl;
            }
        

            waiting_count.fetch_add(1);
            cv_main.notify_one();

            cv_marker.wait(lk, [data] {
                return data->terminate.load() || data->continue_work.load();
                });

            if (data->terminate.load()) {
                for (int idx : marked_indices)
                    (*data->array)[idx] = 0;
                break;
            }

            if (data->continue_work.load()) {
                data->continue_work.store(false);
            }
        }
    }

    waiting_count.fetch_sub(1);
    std::lock_guard<std::mutex> lk(mtx);
    std::cout << "Marker thread " << data->thread_id << " finished.\n";
}

void create_marker_threads(std::vector<MarkerData>& data, std::vector<std::thread>& threads, std::vector<int>& array) {
    for (int i = 0; i < static_cast<int>(data.size()); ++i) {
        data[i].array = &array;
        data[i].thread_id = i + 1;
        threads.emplace_back(marker_thread, &data[i]);
    }
}

void wait_for_all_markers(int active_threads) {
    std::unique_lock<std::mutex> lk(mtx);
    cv_main.wait(lk, [&] { return waiting_count.load() == active_threads; });
}

void terminate_thread(int id, std::vector<MarkerData>& data, std::vector<std::thread>& threads, std::vector<int>& array, int& active_threads) {
    data[id - 1].terminate.store(true);
    cv_marker.notify_all();

    if (threads[id - 1].joinable())
        threads[id - 1].join();

    active_threads--;

    std::ostringstream oss;
    oss << "\nArray after terminating thread " << id << ":";
    print_array(array, oss.str());
}

void terminate_all_threads(std::vector<MarkerData>& data, std::vector<std::thread>& threads, std::vector<int>& array, int& active_threads) {
    std::cout << "\nTerminating ALL threads...\n";

    for (auto& d : data)
        d.terminate.store(true);

    cv_marker.notify_all();

    for (auto& t : threads)
        if (t.joinable()) t.join();

    active_threads = 0;

    print_array(array, "Array after terminating all threads:");
}

void resume_other_markers(std::vector<MarkerData>& data) {
    waiting_count.store(0);
    for (auto& d : data)
        if (!d.terminate.load())
            d.continue_work.store(true);
    cv_marker.notify_all();
}

int choose_thread_to_terminate(const std::vector<MarkerData>& data) {
    int id = -1;
    int max_id = static_cast<int>(data.size());

    while (true) {
        std::cout << "Enter thread to finish ( from 1 to " << max_id << ", or -1 for all): ";
        if (!(std::cin >> id)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        if (id == -1)
            return -1;

        if (id < 1 || id > max_id) {
            std::cout << "Invalid thread ID.\n";
            continue;
        }
        if (data[id - 1].terminate.load()) {
            std::cout << "That thread is already terminated.\n";
            continue;
        }
        break;
    }
    return id;
}
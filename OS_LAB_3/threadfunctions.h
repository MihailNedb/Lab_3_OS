#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>


extern std::mutex mtx;
extern std::condition_variable cv_main;
extern std::condition_variable cv_marker;
extern std::atomic<bool> start_all;
extern std::atomic<int> waiting_count;


struct MarkerData {
    std::vector<int>* array;
    int thread_id;
    std::atomic<bool> terminate{ false };
    std::atomic<bool> continue_work{ false };
    std::atomic<int> marked_count{ 0 };
};


int choose_thread_to_terminate(const std::vector<MarkerData>& data);
void marker_thread(MarkerData* data);
void create_marker_threads(std::vector<MarkerData>& data, std::vector<std::thread>& threads, std::vector<int>& array);
void wait_for_all_markers(int active_threads);
void terminate_thread(int id, std::vector<MarkerData>& data, std::vector<std::thread>& threads, std::vector<int>& array, int& active_threads);
void terminate_all_threads(std::vector<MarkerData>& data, std::vector<std::thread>& threads, std::vector<int>& array, int& active_threads);
void resume_other_markers(std::vector<MarkerData>& data);
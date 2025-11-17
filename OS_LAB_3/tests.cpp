#include "threadfunctions.h"
#include "mathfunctions.h"
#include <gtest/gtest.h>
#include <vector>
#include <sstream>
#include <iostream>
#include <atomic>
#include <thread>

// ------------------------- Тесты для mathfunctions -------------------------

TEST(MathFunctionsTest, PrintArrayDoesNotCrash) {
    std::vector<int> arr1 = {};
    EXPECT_NO_THROW(print_array(arr1, "Empty array:"));

    std::vector<int> arr2 = {1, 2, 3};
    EXPECT_NO_THROW(print_array(arr2, "Filled array:"));
}

TEST(MathFunctionsTest, GetPositiveNumberValidInput) {
    std::istringstream input("5\n");
    std::cin.rdbuf(input.rdbuf());
    EXPECT_EQ(get_positive_number("Enter number: "), 5);
}

TEST(MathFunctionsTest, GetPositiveNumberRetryOnInvalid) {
    std::istringstream input("0\n-5\nabc\n10\n");
    std::cin.rdbuf(input.rdbuf());
    EXPECT_EQ(get_positive_number("Enter number: "), 10);
}


TEST(MathFunctionsTest, GenerateRandomNumberInRange) {
    for (int i = 0; i < 100; ++i) {
        int num = generate_random_number(1, 10);
        EXPECT_GE(num, 1);
        EXPECT_LE(num, 10);
    }
}

TEST(MathFunctionsTest, PrintArrayEmptyVector) {
    testing::internal::CaptureStdout();
    std::vector<int> empty;
    print_array(empty, "Test empty:");
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("Test empty:") != std::string::npos);
}

// ------------------------- Тесты для threadfunctions -------------------------

TEST(ThreadFunctionsTest, MarkerDataInitialization) {
    std::vector<int> array(3, 0);
    MarkerData marker;
    marker.array = &array;
    marker.thread_id = 1;

    EXPECT_FALSE(marker.terminate.load());
    EXPECT_FALSE(marker.continue_work.load());
    EXPECT_EQ(marker.marked_count.load(), 0);
    EXPECT_EQ(marker.thread_id, 1);
}

TEST(ThreadFunctionsTest, MarkerDataFlagsModification) {
    MarkerData marker;
    marker.array = nullptr;
    marker.thread_id = 1;

    marker.terminate.store(true);
    marker.continue_work.store(true);
    marker.marked_count.store(5);

    EXPECT_TRUE(marker.terminate.load());
    EXPECT_TRUE(marker.continue_work.load());
    EXPECT_EQ(marker.marked_count.load(), 5);
}

TEST(ThreadFunctionsTest, SimulatedMarkIndex) {
    std::vector<int> array(5, 0);
    MarkerData marker;
    marker.array = &array;
    marker.thread_id = 1;

   
    int index = 2;
    if ((*marker.array)[index] == 0) {
        (*marker.array)[index] = marker.thread_id;
        marker.marked_count++;
    }

    EXPECT_EQ(array[2], 1);
    EXPECT_EQ(marker.marked_count.load(), 1);

  
    if ((*marker.array)[index] == 0) {
        (*marker.array)[index] = marker.thread_id;
        marker.marked_count++;
    }

    EXPECT_EQ(array[2], 1);  
    EXPECT_EQ(marker.marked_count.load(), 1);  
}



TEST(ThreadFunctionsTest, ChooseThreadToTerminateValidInput) {
    std::vector<MarkerData> data(3);
    for (int i = 0; i < 3; ++i) {
        data[i].thread_id = i + 1;
    }
    
    std::istringstream input("2\n");
    std::cin.rdbuf(input.rdbuf());
    EXPECT_EQ(choose_thread_to_terminate(data), 2);
}

TEST(ThreadFunctionsTest, ChooseThreadToTerminateTerminateAll) {
    std::vector<MarkerData> data(2);
    std::istringstream input("-1\n");
    std::cin.rdbuf(input.rdbuf());
    EXPECT_EQ(choose_thread_to_terminate(data), -1);
}

TEST(ThreadFunctionsTest, ChooseThreadToTerminateRetryOnInvalid) {
    std::vector<MarkerData> data(3);
    std::istringstream input("0\n5\nabc\n2\n");
    std::cin.rdbuf(input.rdbuf());
    EXPECT_EQ(choose_thread_to_terminate(data), 2);
}

TEST(ThreadFunctionsTest, AtomicVariablesInitialState) {
    EXPECT_FALSE(start_all.load());
    EXPECT_EQ(waiting_count.load(), 0);
}

TEST(ThreadFunctionsTest, MutexLockUnlock) {
    std::mutex test_mtx;
    EXPECT_NO_THROW({
        std::lock_guard<std::mutex> lock(test_mtx);
    });
}

TEST(ThreadFunctionsTest, ConditionVariableCreation) {
    std::condition_variable cv;
    std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    
    EXPECT_NO_THROW(cv.notify_one());
    EXPECT_NO_THROW(cv.notify_all());
}

TEST(ThreadFunctionsTest, ResumeOtherMarkersResetsFlags) {
    std::vector<MarkerData> data(2);
    data[0].terminate.store(true);  
    data[1].terminate.store(false); 
    
    resume_other_markers(data);
    
    EXPECT_TRUE(data[1].continue_work.load()); 
    EXPECT_EQ(waiting_count.load(), 0);        
}

TEST(ThreadFunctionsTest, TerminateThreadJoinsCorrectly) {
    std::vector<int> array(5, 0);
    std::vector<MarkerData> data(1);
    std::vector<std::thread> threads;
    
    data[0].array = &array;
    data[0].thread_id = 1;
    
    
    threads.emplace_back([]() { /* пустой поток */ });
    
    int active_threads = 1;
    testing::internal::CaptureStdout();
    terminate_thread(1, data, threads, array, active_threads);
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(active_threads, 0);
    EXPECT_TRUE(data[0].terminate.load());
}

TEST(ThreadFunctionsTest, TerminateAllThreadsSetsFlags) {
    std::vector<MarkerData> data(3);
    std::vector<std::thread> threads;
    std::vector<int> array(10, 0);
    
    for (int i = 0; i < 3; ++i) {
        data[i].thread_id = i + 1;
        threads.emplace_back([]() { /* пустые потоки */ });
    }
    
    int active_threads = 3;
    testing::internal::CaptureStdout();
    terminate_all_threads(data, threads, array, active_threads);
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(active_threads, 0);
    for (const auto& d : data) {
        EXPECT_TRUE(d.terminate.load());
    }
}

TEST(ThreadFunctionsTest, GlobalSyncVariablesIsolation) {
   
    auto initial_waiting = waiting_count.load();
    auto initial_start = start_all.load();
    
    waiting_count.store(5);
    start_all.store(true);
    
    EXPECT_EQ(waiting_count.load(), 5);
    EXPECT_TRUE(start_all.load());
    
    // Сброс для других тестов
    waiting_count.store(initial_waiting);
    start_all.store(initial_start);
}

TEST(ThreadFunctionsTest, MarkerDataArrayPointer) {
    std::vector<int> array1(5, 0);
    std::vector<int> array2(10, 0);
    
    MarkerData marker;
    marker.array = &array1;
    EXPECT_EQ(marker.array->size(), 5);
    
    marker.array = &array2;
    EXPECT_EQ(marker.array->size(), 10);
}

TEST(ThreadFunctionsTest, ThreadIDAssignment) {
    std::vector<MarkerData> data(5);
    for (int i = 0; i < 5; ++i) {
        data[i].thread_id = i + 1;
        EXPECT_EQ(data[i].thread_id, i + 1);
    }
}


TEST(ThreadFunctionsTest, AtomicOperationsThreadSafety) {
    std::atomic<int> counter{0};
    
    auto incrementer = [&counter]() {
        for (int i = 0; i < 1000; ++i) {
            counter.fetch_add(1);
        }
    };
    
    std::thread t1(incrementer);
    std::thread t2(incrementer);
    
    t1.join();
    t2.join();
    
    EXPECT_EQ(counter.load(), 2000);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
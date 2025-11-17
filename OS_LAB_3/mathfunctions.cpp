#include "mathfunctions.h"
#include <iostream>
#include <sstream>
#include <random>
#include <cmath>

int get_positive_number(const std::string& prompt) {
    int n = 0;
    while (true) {
        std::cout << prompt;
        if (std::cin >> n && n > 0) return n;
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "Invalid input. Try again.\n";
    }
}

void print_array(const std::vector<int>& array, const std::string& msg) {
    if (!msg.empty()) std::cout << msg << "\n";
    for (int v : array) std::cout << v << " ";
    std::cout << "\n";
}

int generate_random_number(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}
#pragma once
#include <string>
#include <vector>


int get_positive_number(const std::string& prompt);
void print_array(const std::vector<int>& array, const std::string& msg = "");
int generate_random_number(int min, int max);
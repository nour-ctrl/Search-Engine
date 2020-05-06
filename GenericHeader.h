#pragma once
#ifndef GENERIC_HEADER_H
#define GENERIC_HEADER_H

#include <string>
#include <vector>

template<typename T> 
void PrintVector(const std::vector<T> &v) {// printing 
	std::cout << "[ ";
	for (typename std::vector<T>::const_iterator it = v.begin(); it != v.end(); ++it) {
		std::cout << *it << " ";
	}
	std::cout << "]" << std::endl;
}
void Split(const std::string &s, const char* delim, std::vector<std::string>& v);

// Return true if there is still progress.
bool Progress(std::vector<double> v1, std::vector<double> v2, double thresh);

#endif

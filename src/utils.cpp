#include "pfederc/utils.hpp"

unsigned char pfederc::hash(const std::string &str) {
	if (str.empty()) return 127;
	unsigned char result = str[0];
	for (std::size_t i = 1; i < str.length(); ++i)
		result ^= str[i];

	return result;
}



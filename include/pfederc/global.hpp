#ifndef PFEDERC_GLOBAL_HPP
#define PFEDERC_GLOBAL_HPP

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

#define PFEDERC_PANIC(msg) \
	std::cerr << __FILE__ << ':' << __LINE__ << ": " << msg << std::endl;\
	exit(1)

#endif /* PFEDERC_GLOBAL_HPP */

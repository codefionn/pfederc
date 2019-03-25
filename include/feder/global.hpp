#ifndef FEDER_GLOBAL_HPP
#define FEDER_GLOBAL_HPP

/*!\file feder/global.hpp
 * \brief Include required header files
 */

// Include frome standard library

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#ifndef NSANITY
/*!\brief Perform sanity checks.
 */
#define SANITY
/*!\brief Report fatal error if cond is false.
 * \see feder::fatal
 */
#define FEDER_SANITY_CHECK(cond, msg)                                          \
  if (!(cond))                                                                 \
  feder::fatal(std::string(__FILE__) + ":" + std::to_string(__LINE__) +        \
               ": " + msg)
#else
#define FEDER_SANITY_CHECK(cond, msg)
#endif /* NSANITY */

/*!\brief Incompatible API changes.
 *
 * No guarantee for major version 0.
 */
#define PFEDERC_VERSION_MAJOR 0

/*!\brief Forward-incompatible API changes. Backward-compatible.
 *
 * No guarantee for major version 0.
 */
#define PFEDERC_VERSION_MINOR 0

/*!\brief No API changes.
 *
 * No guarantee for major version 0.
 */
#define PFEDERC_VERSION_REVISION 0

namespace feder {
/*\brief Reports fatal error.
 * \param msg The message to print.
 *
 * Reports fatal error to stderr and terminates program with code 1.
 */
void fatal(const std::string &msg);
} // namespace feder

#endif /* FEDER_GLOBAL_HPP */

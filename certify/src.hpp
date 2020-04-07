#include "certify/detail/config.hpp"

#if BOOST_CERTIFY_HEADER_ONLY
#error src.hpp must not be used in header-only mode, define BOOST_CERTIFY_SEPARATE_COMPILATION
#endif // BOOST_CERTIFY_HEADER_ONLY

#if BOOST_WINDOWS
#include "certify/detail/keystore_windows.ipp"
#elif __APPLE__
#include "certify/detail/keystore_apple.ipp"
#else
#include "certify/detail/keystore_default.ipp"
#endif

#include "certify/impl/https_verification.ipp"

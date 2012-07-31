#include <CGAL/basic.h>
#include "test_configuration.h"
#include <iostream>

#if ((TEST_TRAITS == CORE_CONIC_TRAITS) ||	\
     (TEST_TRAITS == BEZIER_TRAITS) ||					\
     (TEST_TRAITS == RATIONAL_ARC_TRAITS)) && !defined(CGAL_USE_CORE)

int main ()
{
//  bool   UNTESTED_TRAITS_AS_CORE_IS_NOT_INSTALLED;
  std::cout << std::endl
            << "NOTE: Core is not installed, "
            << "skipping the test ..."
            << std::endl;
  return 0;
}
#elif (TEST_TRAITS == ALGEBRAIC_TRAITS) && \
  (TEST_NT == LEDA_INT_NT || TEST_NT == LEDA_RAT_NT) && \
  (! CGAL_USE_LEDA)

int main ()
{
//  bool   UNTESTED_TRAITS_AS_LEDA_IS_NOT_INSTALLED;
  std::cout << std::endl
	    << "NOTE: LEDA is not installed, "
            << "skipping the test ..."
            << std::endl;
  return 0;
}

#elif (TEST_TRAITS == ALGEBRAIC_TRAITS) && \
  (TEST_NT == CGAL_GMPZ_NT || TEST_NT == CGAL_GMPQ_NT) && \
  ! (CGAL_USE_GMP && CGAL_USE_MPFI)

int main ()
{

//  bool   UNTESTED_TRAITS_AS_GMP_OR_MPFI_IS_NOT_INSTALLED;
  std::cout << std::endl
	    << "NOTE: GMP and/or MPFI are not installed, "
            << "skipping the test ..."
            << std::endl;
  return 0;
}

#elif (TEST_TRAITS == ALGEBRAIC_TRAITS) && \
  (TEST_NT == CORE_INT_NT) && \
  !CGAL_USE_CORE

int main ()
{
//  bool   UNTESTED_TRAITS_AS_CORE_IS_NOT_INSTALLED;
  std::cout << std::endl
	    << "NOTE: CORE is not installed, "
            << "skipping the test ..."
            << std::endl;
  return 0;
}


#else

#include "test_traits.h"
#include "Traits_test.h"

int main (int argc, char * argv[])
{
#if TEST_TRAITS == ALGEBRAIC_TRAITS
  CGAL::set_pretty_mode(std::cout);
  CGAL::set_pretty_mode(std::cerr);
#endif

  Traits_test<Traits> test;
  if (!test.parse(argc, argv)) return -1;
  if (!test.init()) return -1;
  if (!test.perform()) return -1;
  return 0;
}

#endif

#define GRB_USE_CUDA

#include <vector>
#include <iostream>

#include "graphblas/graphblas.hpp"
#include "test/test.hpp"

#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE vec_suite

#include <boost/test/included/unit_test.hpp>
#include <boost/program_options.hpp>

void testVector( const std::vector<int>& rhs )
{
  graphblas::Vector<int> vec1(rhs.size());
  graphblas::Index size = rhs.size();
  std::vector<int> lhs;
  CHECKVOID( vec1.build(&rhs, rhs.size()) );
  CHECKVOID( vec1.extractTuples(&lhs, &size) );
  BOOST_ASSERT_LIST( lhs, rhs, rhs.size() );
}

void testNnew( const std::vector<int>& rhs )
{
  graphblas::Vector<int> vec1;
  CHECKVOID( vec1.nnew(rhs.size()) );
  graphblas::Index size = rhs.size();
  std::vector<int> lhs;
  CHECKVOID( vec1.build(&rhs, rhs.size()) );
  CHECKVOID( vec1.extractTuples(&lhs, &size) );
  BOOST_ASSERT_LIST( lhs, rhs, rhs.size() );
}

void testDup( const std::vector<int>& rhs )
{
  graphblas::Vector<int> vec1(rhs.size());
  graphblas::Index size = rhs.size();
  std::vector<int> lhs;
  CHECKVOID( vec1.build(&rhs, rhs.size()) );
  CHECKVOID( vec1.extractTuples(&lhs, &size) );
  BOOST_ASSERT_LIST( lhs, rhs, rhs.size() );

  graphblas::Vector<int> vec2(rhs.size());
  vec2.dup( &vec1 );

  size = rhs.size();
  CHECKVOID( vec2.extractTuples(&lhs, &size) );
  BOOST_ASSERT_LIST( lhs, rhs, rhs.size() );
}

// Test properties:
// 1) vec_type_ is GrB_UNKNOWN
// 2) nvals_ is 0
// 3) nsize_ remains the same
void testClear( const std::vector<int>& rhs )
{
  graphblas::Vector<int> vec1(rhs.size());
  graphblas::Index size = rhs.size();
  std::vector<int> lhs;
  CHECKVOID( vec1.build(&rhs, rhs.size()) );
  CHECKVOID( vec1.extractTuples(&lhs, &size) );
  BOOST_ASSERT_LIST( lhs, rhs, rhs.size() );
  
  CHECKVOID( vec1.clear() );
  lhs.clear();
  CHECKVOID( vec1.size(&size) );
  BOOST_ASSERT( size==rhs.size() );

  graphblas::Index nvals;
  CHECKVOID( vec1.nvals(&nvals) );
  BOOST_ASSERT( nvals==0 );

  graphblas::Storage storage;
  CHECKVOID( vec1.getStorage(&storage) );
  BOOST_ASSERT( storage==graphblas::GrB_UNKNOWN );
}

void testSize( const std::vector<int>& rhs )
{
  graphblas::Vector<int> vec1(rhs.size());
  graphblas::Index size = rhs.size();
  std::vector<int> lhs;
  CHECKVOID( vec1.build(&rhs, rhs.size()) );
  CHECKVOID( vec1.extractTuples(&lhs, &size) );
  BOOST_ASSERT_LIST( lhs, rhs, rhs.size() );
  
  CHECKVOID( vec1.size(&size) );
  BOOST_ASSERT( size==rhs.size() );
}

void testNvals( const std::vector<int>& rhs )
{
  graphblas::Vector<int> vec1(rhs.size());
  graphblas::Index size = rhs.size();
  std::vector<int> lhs;
  CHECKVOID( vec1.build(&rhs, rhs.size()) );
  CHECKVOID( vec1.extractTuples(&lhs, &size) );
  BOOST_ASSERT_LIST( lhs, rhs, rhs.size() );
  
  CHECKVOID( vec1.nvals(&size) );
  BOOST_ASSERT( size==rhs.size() );
}

void testBuild( const std::vector<int>& rhs )
{
  graphblas::Vector<int> vec1(rhs.size());
  graphblas::Index size = rhs.size();
  std::vector<int> lhs;
  CHECKVOID( vec1.build(&rhs, rhs.size()) );
  CHECKVOID( vec1.extractTuples(&lhs, &size) );
  BOOST_ASSERT_LIST( lhs, rhs, rhs.size() );
}

void testSetElement( std::vector<int>& rhs, int val, int ind )
{
  graphblas::Vector<int> vec1(rhs.size());
  graphblas::Index size = rhs.size();
  std::vector<int> lhs;
  CHECKVOID( vec1.build(&rhs, rhs.size()) );
  CHECKVOID( vec1.setElement(val, ind) );
  CHECKVOID( vec1.extractTuples(&lhs, &size) );
  rhs[ind] = val;
  BOOST_ASSERT_LIST( lhs, rhs, rhs.size() );
}

void testExtractElement( const std::vector<int>& rhs )
{
}

void testExtractTuples( const std::vector<int>& rhs )
{
  graphblas::Vector<int> vec1(rhs.size());
  graphblas::Index size = rhs.size();
  std::vector<int> lhs;
  CHECKVOID( vec1.build(&rhs, rhs.size()) );
  CHECKVOID( vec1.extractTuples(&lhs, &size) );
  BOOST_ASSERT_LIST( lhs, rhs, rhs.size() );
}

void testOperatorGetElement( const std::vector<int>& rhs )
{
}

void testFill( int size )
{
  graphblas::Vector<int> vec1(size);
  std::vector<int> lhs;
  std::vector<int> rhs(size, 5);
  CHECKVOID( vec1.fill(5) );
  CHECKVOID( vec1.extractTuples(&lhs, &size) );
  BOOST_ASSERT_LIST( lhs, rhs, size );
}

void testFillAscending( int size )
{
  graphblas::Vector<int> vec1(size);
  std::vector<int> lhs;
  std::vector<int> rhs(size);
  std::iota( std::begin(rhs), std::end(rhs), 0);
  CHECKVOID( vec1.fillAscending(size) );
  CHECKVOID( vec1.extractTuples(&lhs, &size) );
  BOOST_ASSERT_LIST( lhs, rhs, size );
}

void testResize( const std::vector<int>& rhs, const int nvals )
{
  graphblas::Vector<int> vec1(rhs.size());
  graphblas::Index size = rhs.size();
  std::vector<int> lhs;
  CHECKVOID( vec1.build(&rhs, rhs.size()) );
  CHECKVOID( vec1.extractTuples(&lhs, &size) );
  BOOST_ASSERT_LIST( lhs, rhs, rhs.size() );

  CHECKVOID( vec1.resize(nvals) );
  size = rhs.size();
  CHECKVOID( vec1.extractTuples(&lhs, &size) );
  BOOST_ASSERT_LIST( lhs, rhs, rhs.size() );

  CHECKVOID( vec1.size(&size) );
  BOOST_ASSERT( size==nvals );
  CHECKVOID( vec1.nvals(&size) );
  BOOST_ASSERT( size==nvals );
}

void testSwap( const std::vector<int>& lhs, const std::vector<int>& rhs )
{
  graphblas::Vector<int> vec1(lhs.size());
  graphblas::Index lhs_size = lhs.size();
  std::vector<int> lhs_val;
  CHECKVOID( vec1.build(&lhs, lhs.size()) );

  graphblas::Vector<int> vec2(rhs.size());
  graphblas::Index rhs_size = rhs.size();
  std::vector<int> rhs_val;
  CHECKVOID( vec2.build(&rhs, rhs.size()) );

  CHECKVOID( vec1.swap(&vec2) );
  CHECKVOID( vec1.extractTuples(&lhs_val, &rhs_size) ); 
  CHECKVOID( vec2.extractTuples(&rhs_val, &lhs_size) ); 
  
  CHECKVOID( vec1.print() );
  CHECKVOID( vec2.print() );
  BOOST_ASSERT_LIST( lhs_val, rhs, rhs_size );
  BOOST_ASSERT_LIST( rhs_val, lhs, lhs_size );
}

struct TestVector
{
  TestVector() :
    DEBUG(true) {}

  bool DEBUG;
};

BOOST_AUTO_TEST_SUITE( vec_suite )

BOOST_FIXTURE_TEST_CASE( vec1, TestVector )
{
  std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  testVector( vec );
  testNnew(   vec );
  testDup(    vec );
  testClear(  vec );
  testSize(   vec );
  testNvals(  vec );
  testBuild(  vec );
  testSetElement(     vec, 4, 5 );
  testExtractTuples(  vec );
  testFill(           20 );
  testFillAscending(  20 );
}

BOOST_FIXTURE_TEST_CASE( vec2, TestVector )
{
  std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 0, 2};
  testResize( vec, 15 );
}

BOOST_FIXTURE_TEST_CASE( vec3, TestVector )
{
  std::vector<int> vec1 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  std::vector<int> vec2 = {1, 2, 3, 4, 5, 6, 7, 8, 0, 2, 11};
  testSwap( vec1, vec2 );
}

BOOST_AUTO_TEST_SUITE_END()

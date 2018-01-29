#ifndef GRB_BACKEND_APSPIE_OPERATIONS_HPP
#define GRB_BACKEND_APSPIE_OPERATIONS_HPP

#include "graphblas/backend/apspie/spgemm.hpp"
#include "graphblas/backend/apspie/spmm.hpp"
#include "graphblas/backend/apspie/gemm.hpp"
#include "graphblas/backend/apspie/spmspv.hpp"
#include "graphblas/backend/apspie/spmv.hpp"
#include "graphblas/backend/apspie/gemv.hpp"
#include "graphblas/backend/apspie/assign.hpp"
#include "graphblas/backend/apspie/reduce.hpp"
#include "graphblas/backend/apspie/Descriptor.hpp"

namespace graphblas
{
namespace backend
{
  template <typename T>
  class Vector;

  template <typename T>
  class Matrix;

  template <typename T1, typename T2, typename T3>
  class BinaryOp;
  
  template <typename T>
  class Monoid;

  template <typename T1, typename T2, typename T3>
  class Semiring;

  template <typename c, typename a, typename b, typename m,
            typename BinaryOpT,     typename SemiringT>
  Info mxm( Matrix<c>*       C,
            const Matrix<m>* mask,
            const BinaryOpT* accum,
            const SemiringT* op,
            const Matrix<a>* A,
            const Matrix<b>* B,
            Descriptor*      desc )
  {
    Storage A_mat_type;
    Storage B_mat_type;
    CHECK( A->getStorage( &A_mat_type ) );
    CHECK( B->getStorage( &B_mat_type ) );

    if( A_mat_type==GrB_SPARSE && B_mat_type==GrB_SPARSE )
    {
      CHECK( C->setStorage( GrB_SPARSE ) );
      CHECK( spgemm( C->getMatrix(), mask, accum, op, A->getMatrix(), 
          B->getMatrix(), desc ) );
    }
    else
    {
      CHECK( C->setStorage( GrB_DENSE ) );
      if( A_mat_type==GrB_SPARSE && B_mat_type==GrB_DENSE )
      {
        CHECK( spmm( C->getMatrix(), mask, accum, op, A->getMatrix(), 
            B->getMatrix(), desc ) );
      }
      else if( A_mat_type==GrB_DENSE && B_mat_type==GrB_SPARSE )
      {
        CHECK( spmm( C->getMatrix(), mask, accum, op, A->getMatrix(), 
            B->getMatrix(), desc ) );
      }
      else
      {
        CHECK( gemm( C->getMatrix(), mask, accum, op, A->getMatrix(), 
            B->getMatrix(), desc ) );
      }
    }
    return GrB_SUCCESS;
  }

  template <typename W, typename U, typename a>
  Info vxm( Vector<W>*             w,
            const Vector<U>*       mask,
            const BinaryOp<a,a,a>* accum,
            const Semiring<a,a,a>* op,
            const Vector<U>*       u,
            const Matrix<a>*       A,
            Descriptor*            desc )
  {
    // Get storage:
    Storage u_vec_type;
    Storage A_mat_type;
    CHECK( u->getStorage( &u_vec_type ) );
    CHECK( A->getStorage( &A_mat_type ) );

    // Conversions:
    Desc_value vxm_mode, tol;
    CHECK( desc->get( GrB_MXVMODE, &vxm_mode ) );

    // Note: removed tol for now
    //CHECK( desc->get( GrB_TOL,     &tol      ) );
    Vector<U>* u_t = const_cast<Vector<U>*>(u);

    // Note here, the 1.f stands for the dense vector one-element (STRUCONLY)
    // The op->identity() stands for the dense vector zero-element
    // TODO: Fix this hack
    if( vxm_mode==GrB_PUSHPULL )
      CHECK( u_t->convert( 0.f, 1.f, desc ) );
    else if( vxm_mode==GrB_PUSHONLY && u_vec_type==GrB_DENSE )
      CHECK( u_t->dense2sparse( 0.f, desc ) );
    else if( vxm_mode==GrB_PULLONLY && u_vec_type==GrB_SPARSE )
      CHECK( u_t->sparse2dense( 0.f, 1.f, desc ) );

    // Check if vector type was changed due to conversion!
    CHECK( u->getStorage( &u_vec_type ) );

    // Transpose:
    Desc_value inp0_mode;
    CHECK( desc->get(GrB_INP0, &inp0_mode) );
    if( inp0_mode!=GrB_DEFAULT ) return GrB_INVALID_VALUE;

    // Treat vxm as an mxv with transposed matrix
	  CHECK( desc->toggle( GrB_INP1 ) );

    // Breakdown into 3 cases:
    // 1) SpMSpV: SpMat x SpVe
    // 2) SpMV:   SpMat x DeVec
    // 3) GeMV:   DeMat x DeVec
    if( A_mat_type==GrB_SPARSE && u_vec_type==GrB_SPARSE )
    {
      CHECK( w->setStorage( GrB_SPARSE ) );
      CHECK( spmspv( &w->sparse_, mask, accum, op, &A->sparse_, 
          &u->sparse_, desc ) );
    }
    else
    {
      CHECK( w->setStorage( GrB_DENSE ) );
      if( A_mat_type==GrB_SPARSE )
        CHECK(spmv(&w->dense_, mask, accum, op, &A->sparse_, &u->dense_, desc));
      else
        CHECK(gemv( &w->dense_, mask, accum, op, &A->dense_, &u->dense_, desc));
    }

    // Undo change to desc by toggling again
	  CHECK( desc->toggle( GrB_INP1 ) );

    return GrB_SUCCESS;
  }

  // Only difference between vxm and mxv is an additional check for gemv
  // to transpose A 
  // -this is because w=uA is same as w=A^Tu
  // -i.e. GraphBLAS treats 1xn Vector the same as nx1 Vector
  template <typename W, typename a, typename U>
  Info mxv( Vector<W>*       w,
            const Vector<U>* mask,
            const BinaryOp<a,a,a>* accum,
            const Semiring<a,a,a>* op,
            const Matrix<a>* A,
            const Vector<U>* u,
            Descriptor*      desc )
  {
    // Get storage:
    Storage u_vec_type;
    Storage A_mat_type;
    CHECK( u->getStorage( &u_vec_type ) );
    CHECK( A->getStorage( &A_mat_type ) );

    // Conversions:
    Desc_value mxv_mode;
    Desc_value tol;
    CHECK( desc->get( GrB_MXVMODE, &mxv_mode ) );
    CHECK( desc->get( GrB_TOL,     &tol      ) );
    Vector<U>* u_t = const_cast<Vector<U>*>(u);
    if( mxv_mode==GrB_PUSHPULL )
      CHECK( u_t->convert( 0.f, 1.f, desc ) );
    else if( mxv_mode==GrB_PUSHONLY && u_vec_type==GrB_DENSE )
      CHECK( u_t->dense2sparse( 0.f, desc ) );
    else if( mxv_mode==GrB_PULLONLY && u_vec_type==GrB_SPARSE )
      CHECK( u_t->sparse2dense( 0.f, 1.f, desc ) );

    // Check if vector type was changed due to conversion!
    CHECK( u->getStorage( &u_vec_type ) );

    // Transpose:
    Desc_value inp1_mode;
    CHECK( desc->get(GrB_INP1, &inp1_mode) );
    if( inp1_mode!=GrB_DEFAULT ) return GrB_INVALID_VALUE;

    // 3 cases:
    // 1) SpMSpV: SpMat x SpVe
    // 2) SpMV:   SpMat x DeVec
    // 3) GeMV:   DeMat x DeVec
    if( A_mat_type==GrB_SPARSE && u_vec_type==GrB_SPARSE )
    {
      CHECK( w->setStorage( GrB_SPARSE ) );
      CHECK( spmspv( &w->sparse_, mask, accum, op, &A->sparse_, 
          &u->sparse_, desc ) );
    }
    else
    {
      CHECK( w->setStorage( GrB_DENSE ) );
      if( A_mat_type==GrB_SPARSE )
      {
        CHECK( spmv( &w->dense_, mask, accum, op, &A->sparse_, 
            &u->dense_, desc ) );
      }
      else
      {
        CHECK( gemv( &w->dense_, mask, accum, op, &A->dense_, 
            &u->dense_, desc ) );
      }
    }
    return GrB_SUCCESS;
  }

  template <typename W, typename U, typename V, typename M,
            typename BinaryOpT,     typename SemiringT>
  Info eWiseMult( Vector<W>*       w,
                  const Vector<M>* mask,
                  const BinaryOpT* accum,
                  const SemiringT* op,
                  const Vector<U>* u,
                  const Vector<V>* v,
                  Descriptor*      desc )
  {
    // Use either op->operator() or op->mul() as the case may be
  }

  template <typename c, typename a, typename b, typename m,
            typename BinaryOpT,     typename SemiringT>
  Info eWiseMult( Matrix<c>*       C,
                  const Matrix<m>* mask,
                  const BinaryOpT* accum,
                  const SemiringT* op,
                  const Matrix<a>* A,
                  const Matrix<b>* B,
                  Descriptor*      desc )
  {
    // Use either op->operator() or op->mul() as the case may be
  }

  template <typename W, typename U, typename V, typename M,
            typename BinaryOpT,     typename SemiringT>
  Info eWiseAdd( Vector<W>*       w,
                 const Vector<M>* mask,
                 const BinaryOpT* accum,
                 const SemiringT* op,
                 const Vector<U>* u,
                 const Vector<V>* v,
                 Descriptor*      desc )
  {
    // Use either op->operator() or op->add() as the case may be
  }

  template <typename c, typename a, typename b, typename m,
            typename BinaryOpT,     typename SemiringT>
  Info eWiseAdd( Matrix<c>*       C,
                 const Matrix<m>* mask,
                 const BinaryOpT* accum,
                 const SemiringT* op,
                 const Matrix<a>* A,
                 const Matrix<b>* B,
                 Descriptor*      desc )
  {
    // Use either op->operator() or op->add() as the case may be
  }

  template <typename W, typename U, typename M,
            typename BinaryOpT>
  Info extract( Vector<W>*                w,
                const Vector<M>*          mask,
                const BinaryOpT*          accum,
                const Vector<U>*          u,
                const std::vector<Index>* indices,
                Index                     nindices,
                Descriptor*               desc )
  {

  }

  template <typename c, typename a, typename m,
            typename BinaryOpT>
  Info extract( Matrix<c>*                C,
                const Matrix<m>*          mask,
                const BinaryOpT*          accum,
                const Matrix<a>*          A,
                const std::vector<Index>* row_indices,
                Index                     nrows,
                const std::vector<Index>* col_indices,
                Index                     ncols,
                Descriptor*               desc )
  {

  }

  template <typename W, typename a, typename M,
            typename BinaryOpT>
  Info extract( Vector<W>*                w,
                const Vector<M>*          mask,
                const BinaryOpT*          accum,
                const Matrix<a>*          A,
                const std::vector<Index>* row_indices,
                Index                     nrows,
                Index                     col_index,
                Descriptor*               desc )
  {

  }

  template <typename W, typename U, typename M,
            typename BinaryOpT>
  Info assign( Vector<W>*                w,
               const Vector<M>*          mask,
               const BinaryOpT*          accum,
               const Vector<U>*          u,
               const std::vector<Index>* indices,
               Index                     nindices,
               Descriptor*               desc )
  {
    return GrB_SUCCESS;
  }

  template <typename c, typename a, typename m,
            typename BinaryOpT>
  Info assign( Matrix<c>*                C,
               const Matrix<m>*          mask,
               const BinaryOpT*          accum,
               const Matrix<a>*          A,
               const std::vector<Index>* row_indices,
               Index                     nrows,
               const std::vector<Index>* col_indices,
               Index                     ncols,
               Descriptor*               desc )
  {

  }

  template <typename c, typename U, typename M,
            typename BinaryOpT>
  Info assign( Matrix<c>*                C,
               const Vector<M>*          mask,
               const BinaryOpT*          accum,
               const Vector<U>*          u,
               const std::vector<Index>* row_indices,
               Index                     nrows,
               Index                     col_index,
               Descriptor*               desc )
  {

  }

  template <typename c, typename U, typename M,
            typename BinaryOpT>
  Info assign( Matrix<c>*                C,
               const Vector<M>*          mask,
               const BinaryOpT*          accum,
               const Vector<U>*          u,
               Index                     row_index,
               const std::vector<Index>* col_indices,
               Index                     ncols,
               Descriptor*               desc )
  {

  }

  template <typename W, typename T, typename M>
  Info assign( Vector<W>*                w,
               const Vector<M>*          mask,
               const BinaryOp<W,W,W>*    accum,
               T                         val,
               const std::vector<Index>* indices,
               Index                     nindices,
               Descriptor*               desc )
  {
    // Get storage:
    Storage vec_type;
    CHECK( w->getStorage( &vec_type ) );

    // 2 cases:
    // 1) SpVec
    // 2) DeVec
    if( vec_type==GrB_SPARSE )
    {
      CHECK( w->setStorage( GrB_SPARSE ) );
      CHECK( assignSparse( &w->sparse_, mask, accum, val, indices, nindices, 
          desc) );
    }
    else if( vec_type==GrB_DENSE )
    {
      CHECK( w->setStorage( GrB_DENSE ) );
      CHECK( assignDense( &w->dense_, mask, accum, val, indices, nindices, 
          desc) );
    }

    return GrB_SUCCESS;
  }

  template <typename c, typename T, typename m,
            typename BinaryOpT>
  Info assign( Matrix<c>*                C,
               const Matrix<m>*          mask,
               const BinaryOpT*          accum,
               T                         val,
               const std::vector<Index>* row_indices,
               Index                     nrows,
               const std::vector<Index>* col_indices,
               Index                     ncols,
               Descriptor*               desc )
  {

  }

  template <typename W, typename U, typename M,
            typename BinaryOpT,     typename UnaryOpT>
  Info apply( Vector<W>*       w,
              const Vector<M>* mask,
              const BinaryOpT* accum,
              const UnaryOpT*  op,
              const Vector<U>* u,
              Descriptor*      desc )
  {

  }

  template <typename c, typename a, typename m,
            typename BinaryOpT,     typename UnaryOpT>
  Info apply( Matrix<c>*       C,
              const Matrix<m>* mask,
              const BinaryOpT* accum,
              const UnaryOpT*  op,
              const Matrix<a>* A,
              Descriptor*      desc )
  {

  }

  template <typename W, typename a, typename M,
            typename BinaryOpT,     typename MonoidT>
  Info reduce( Vector<W>*       w,
               const Vector<M>* mask,
               const BinaryOpT* accum,
               const MonoidT*   op,
               const Matrix<a>  A,
               Descriptor*      desc )
  {
    // Use op->operator()
  }

  template <typename T, typename U>
  Info reduce( T*                     val,
               const BinaryOp<U,U,U>* accum,
               const Monoid<U>*       op,
               const Vector<U>*       u,
               Descriptor*            desc )
  {
    // Get storage:
    Storage vec_type;
    CHECK( u->getStorage( &vec_type ) );

    // 2 cases:
    // 1) SpVec
    // 2) DeVec
    if( vec_type==GrB_SPARSE )
    {
      CHECK( reduceInner( val, accum, op, &u->sparse_, desc) );
    }
    else if( vec_type==GrB_DENSE )
    {
      CHECK( reduceInner( val, accum, op, &u->dense_, desc) );
    }

    return GrB_SUCCESS;
  }

  template <typename T, typename a,
            typename BinaryOpT,     typename MonoidT>
  Info reduce( T*                val,
               const BinaryOpT*  accum,
               const MonoidT*    op,
               const Matrix<a>*  A,
               Descriptor*       desc )
  {

  }

  template <typename c, typename a, typename m,
            typename BinaryOpT>
  Info transpose( Matrix<c>*        C,
                  const Matrix<m>*  mask,
                  const BinaryOpT*  accum,
                  const Matrix<a>*  A,
                  Descriptor*       desc )
  {

  }

}  // backend
}  // graphblas

#endif  // GRB_BACKEND_APSPIE_OPERATIONS_HPP

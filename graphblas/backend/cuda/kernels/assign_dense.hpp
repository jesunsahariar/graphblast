#ifndef GRAPHBLAS_BACKEND_CUDA_KERNELS_ASSIGN_DENSE_HPP_
#define GRAPHBLAS_BACKEND_CUDA_KERNELS_ASSIGN_DENSE_HPP_

namespace graphblas {
namespace backend {

// This is the dense mask variant of assignDense
template <bool UseScmp, bool UseMask, bool UseAll,
          typename U, typename M, typename BinaryOpT>
__global__ void assignDenseDenseMaskedKernel(U*           u_val,
                                             Index        u_nvals,
                                             const M*     mask_val,
                                             BinaryOpT    accum_op,
                                             U            val,
                                             const Index* indices,
                                             Index        nindices) {
  Index row = blockIdx.x*blockDim.x + threadIdx.x;

  if (UseAll) {
    for (; row < u_nvals; row += gridDim.x*blockDim.x) {
      if (UseMask) {
        M m_val = mask_val[row];

        // val is assigned if either:
        // 1) UseScmp is not selected and m_val is zero
        // 2) UseScmp is selected and m_val is not zero
        if ((UseScmp && m_val == 0) || (!UseScmp && m_val != 0))
          u_val[row] = val;
      } else {
        u_val[row] = val;
      }
    }
  } else {
    if (row == 0) {
      printf("Selective Indices DeVec Assign Constant Kernel\n");
      printf("Error: Feature not implemented yet!\n");
    }
  }
}

// This is the sparse mask variant of assignDense
// Iterating along u_ind and u_val is better
template <bool UseScmp, bool UseMask, bool UseAll,
          typename U, typename BinaryOpT>
__global__ void assignDenseSparseMaskedKernel(U*           u_val,
                                              Index        u_nvals,
                                              const Index* mask_ind,
                                              Index        mask_nvals,
                                              BinaryOpT    accum_op,
                                              U            val,
                                              const Index* indices,
                                              Index        nindices) {
  Index row = blockIdx.x*blockDim.x + threadIdx.x;

  if (UseAll) {
    if (UseMask) {
      if (UseScmp) {
        if (row == 0) {
          // TODO(@ctcyang): requires binary searching the mask array
          printf("All Indices DeVec Assign Constant Scmp Kernel\n");
          printf("Error: Feature not implemented yet!\n");
        }
      } else {
        for (; row < mask_nvals; row += gridDim.x*blockDim.x) {
          Index ind  = mask_ind[row];
          u_val[ind] = val;
        }
      }
    } else {
      for (; row < u_nvals; row += gridDim.x*blockDim.x)
        u_val[row] = val;
    }
  } else {
    if (row == 0) {
      printf("Selective Indices DeVec Assign Constant Kernel\n");
      printf("Error: Feature not implemented yet!\n");
    }
  }
}
}  // namespace backend
}  // namespace graphblas

#endif  // GRAPHBLAS_BACKEND_CUDA_KERNELS_ASSIGN_DENSE_HPP_

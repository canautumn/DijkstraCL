//***************************************************************************************
//
//! \file DijkstraCL.h
//!  A simple wrapper class for the OpenCL implementation of the Parallel Dijkstra Algorithm
//! for all-pair or selected-pair shortest distances calculation.
//!
//! This code is adapted from the example code (by Dan Ginsburg) for the book
//! OpenCL Programming Guide (by Munish, et al.)
//! (http://www.heterogeneouscompute.org/?page_id=5)
//! with some tweaks, one data reading synchronization bug fixed for large datasets,
//! and multiple source shortest distances support added.
//! Both of this code and the original code implemented the parallel Dijkstra Algorithm from
//! the paper "Accelerating large graph algorithms on the GPU using CUDA" (by Harish and Narayanan).
//!
//! \author    Can
//! \version   1.0
//! \date      2014-08-16
//! \copyright GNU Public License V3.0
//
//***************************************************************************************

#ifndef DIJKSTRACL_DIJKSTRACL_H
#define DIJKSTRACL_DIJKSTRACL_H

#ifdef __APPLE__
#include "OpenCL/opencl.h"
#else
#include "CL/cl.h"
#endif
#include <iostream>
#include "AdjacencyList.h"
#include "Matrix.h"

//! checkError macro to incorporate the line of code with error for convenient debugging.
#define checkError() checkErrorFileLine(__FILE__ , __LINE__)

namespace Dijkstra {

//!  A simple wrapper class for the OpenCL implementation of the Parallel Dijkstra Algorithm
//! for all-pair or selected-pair shortest distances calculation.
class DijkstraCL {
 public:
  typedef int cl_Index; //!< Size type for OpenCL. Check the hardware for supported types.
  typedef float cl_Scalar; //!< Value type for OpenCL. Check the hardware for supported types.

  //! Constructor.
  //! Create the shortest path problem from a predefined GraphArray shared pointer.
  //! The size type and value type of the GraphArray must be compatible with cl_Index and cl_Scalar.
  DijkstraCL(std::shared_ptr<UndirectedWeightedGraph::GraphArray<cl_Index, cl_Scalar>> graph);

  //! Run the parallel shortest distance calculation with default or predefined parameters.
  //! The default behavior is to calculate all-pair shortest distance matrix using one GPU with the
  //! maximum FLOPS.
  cl_int Run();

  //! Get the result array pointer.
  //! The result is a row-major linear storage of the distance matrix.
  std::shared_ptr<std::vector<cl_Scalar>> GetResultsArray();
 private:
  std::shared_ptr<UndirectedWeightedGraph::GraphArray<cl_Index, cl_Scalar>> graph_; //!< Predefined GraphArray shared pointer.
  cl_Index numVertices_; //!< number of vertices of the graph, calculated from graph_.
  std::vector<cl_Index> sourceVertices_; //!< the source vertices list from which the shortest distances to all vertices are calculated.
  std::shared_ptr<std::vector<cl_Scalar>> results_; //!< the result array pointer.
  cl_platform_id platform_; //!< the selected OpenCL platform.
  cl_context context_; //!< the selected OpenCL context.
  cl_device_id device_; //!< the selected OpenCL device.
  cl_int lastErr_; //!< the last error number from OpenCL C API.

  //! Initialize the selected OpenCL devices.
  cl_int InitializeDevices();

  //! Select one device from the device list with the maximum FLOPS.
  cl_device_id GetMaxFlopsDevice();

  //! Get the rounded-up of the global work size to be enough to cover all the global computing elements.
  //! \param groupSize the number of computing elements of one workgroup.
  //! This is obtained by clGetDeviceInfo and depending on the hardware.
  //! \param globalSize the number of global computing elements.
  cl_Index RoundUpWorkSize(cl_Index groupSize, cl_Index globalSize);

  //! Simple helper function to check if all the elements of the mask array are zero (empty) using a loop.
  //! This function works on host memory.
  //! \param mask mask array pointer in host memory.
  //! \param maskSize the size of the mask array.
  bool isMaskEmpty(cl_Index * mask, cl_Index maskSize);

  //! Prepare the device memory.
  //! This function allocates the required device memory, create the host memory pointers, copy the graph data
  //! from host memory to device memory.
  //! \param context the OpenCL context.
  //! \param commandQueue the OpenCL command queue to enqueue the allocating and copy operations.
  //! \param graph the graph array to be copied.
  //! \param verticesDevice the pointer to where a device memory reference for vertices array is created.
  //! \param edgesDevice the pointer to where a device memory reference for edges array is created.
  //! \param weightsDevice the pointer to where a device memory reference for weights array is created.
  //! \param maskDevice the pointer to where a device memory reference for mask array is created.
  //! \param costDevice the pointer to where a device memory reference for cost array is created.
  //! \param updatingCostDevice the pointer to where a device memory reference for updating cost array is created.
  //! \param globalWorkSize rounded-up global work size.
  void PrepareDeviceMemory
      (cl_context context, cl_command_queue commandQueue, UndirectedWeightedGraph::GraphArray<cl_Index, cl_Scalar> &graph,
       cl_mem *verticesDevice, cl_mem *edgesDevice, cl_mem *weightsDevice,
       cl_mem *maskDevice, cl_mem *costDevice, cl_mem *updatingCostDevice,
       size_t globalWorkSize);

  //! Check if the returning error number of OpenCL C API is success;
  //! if not, show the current source file and line number, and terminate the program.
  //! There is no need to call this function directly. This works with the checkError() macro.
  void checkErrorFileLine(const char *file, const int lineNumber);

  //! Build the OpenCL kernel program on the specified context.
  //! \param context the OpenCL context on which the program is built.
  cl_program buildProgram(cl_context context);

  //! Run the OpenCL kernel program on specified context and device.
  //! \param context the OpenCL context on which the program runs.
  //! \param device the OpenCL device on which the program runs.
  cl_int RunOnDevice(cl_context context, cl_device_id device);
};

} // namespace Dijkstra

#endif //DIJKSTRACL_DIJKSTRACL_H

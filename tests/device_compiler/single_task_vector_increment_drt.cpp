/* RUN: %{execute}%s

   A simple typical FPGA-like kernel
*/
#include <CL/sycl.hpp>
#include <iostream>
#include <numeric>

#include <boost/test/minimal.hpp>

using namespace cl::sycl;

constexpr size_t N = 300;
using Type = int;


int test_main(int argc, char *argv[]) {
  buffer<Type> input { N };
  buffer<Type> output { N };

  {
    auto a_input = input.get_access<access::mode::write>();
    // Initialize buffer with increasing numbers starting at 0
    std::iota(a_input.begin(), a_input.end(), 0);
  }

  /// \todo implement queue construction from device_selector
  // Create an OpenCL queue to launch the kernel
  auto oq = boost::compute::system::default_queue();
  queue q { oq };

  // Launch a kernel to do the summation
  q.submit([&] (handler &cgh) {
      // Get access to the data
      auto a_input = input.get_access<access::mode::read>(cgh);
      auto x_output = output.get_access<access::mode::write>(cgh);

      // A typical FPGA-style pipelined kernel
      cgh.single_task<class add>([=,
                                  x_output = drt::accessor<decltype(x_output)> { x_output },
                                  a_input = drt::accessor<decltype(a_input)> { a_input }] {
          for (unsigned int i = 0 ; i < N; ++i)
            x_output[i] = a_input[i] + 42;
        });
    });

  // Verify the result
  auto a_input = input.get_access<access::mode::read>();
  auto a_output = output.get_access<access::mode::read>();
  for (unsigned int i = 0 ; i < input.get_count(); ++i)
    BOOST_CHECK(a_output[i] == a_input[i] + 42);

  return 0;
}
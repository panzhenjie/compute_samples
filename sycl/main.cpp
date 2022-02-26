#include <CL/sycl.hpp>
#include <iostream>

using namespace sycl;

int main() {
	queue q;
	int input0_h[5] = {1, 2, 3, 4, 5};
	int input1_h[5] = {6, 7, 8, 9, 10};
	int output_h[5];
	int *input0_d = malloc_device<int>(5, q);
	int *input1_d = malloc_device<int>(5, q);
	int *output_d = malloc_device<int>(5, q);
	
	q.memcpy(input0_d, input0_h, 5 * sizeof(int));
	q.memcpy(input1_d, input1_h, 5 * sizeof(int));

	q.submit([&](handler& h) {
		h.parallel_for(5, [=](auto& id) {
			output_d[id] = input0_d[id] + input1_d[id];
		});
	});

	q.memcpy(output_h, output_d, 5 * sizeof(int));
	q.wait();
	std::cout << output_h[0] << " " << output_h[1] << " " << output_h[2] << " " << output_h[3] << " " << output_h[4] << std::endl;
	return 0;
}
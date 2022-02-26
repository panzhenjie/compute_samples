#include <stdio.h>
#include <CL/cl.h>

void main()
{
	int input0[5] = {1, 2, 3, 4, 5};
	int input1[5] = {6, 7, 8, 9, 10};
	int output[5];

	cl_int ret;

	cl_platform_id platform;
	ret = clGetPlatformIDs(1, &platform, NULL);
	if (ret != CL_SUCCESS) {
		printf("clGetPlatformIDs() failed\n");
		return;
	}

	cl_device_id device;
	ret = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if (ret != CL_SUCCESS) {
		printf("clGetDeviceIDs() failed\n");
		return;
	}

	cl_context context;
	cl_context_properties props[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0};
	context = clCreateContext(props, 1, &device, NULL, NULL, &ret);
	if (ret != CL_SUCCESS) {
		printf("clCreateContext() failed\n");
		return;
	}

	cl_command_queue queue;
	queue = clCreateCommandQueueWithProperties(context, device, NULL, &ret);
	if (ret != CL_SUCCESS) {
		printf("clCreateCommandQueue() failed\n");
		return;
	}

	FILE *fd = fopen("kernel.cl", "r");
	fseek(fd, 0, SEEK_END);
	size_t len = ftell(fd);
	rewind(fd);
	char *buf = (char *)malloc(len);
	len = fread(buf, sizeof(char), len, fd);
	fclose(fd);

	cl_program program;
	program = clCreateProgramWithSource(context, 1, (const char **)&buf, &len, &ret);
	if (ret != CL_SUCCESS) {
		printf("clCreateProgramWithSource() failed\n");
		return;
	}

	ret = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (ret != CL_SUCCESS) {
		printf("clBuildProgram() failed\n");

		char log[16384];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(log), log, NULL);
		printf("%s", log);

		return;
	}

	cl_kernel kernel;
	kernel = clCreateKernel(program, "add", &ret);
	if (ret != CL_SUCCESS) {
		printf("clCreateKernel() failed\n");
		return;
	}

	cl_mem mem[3];
	mem[0] = clCreateBuffer(context, CL_MEM_READ_ONLY, 5 * sizeof(int), NULL, &ret);
	if (ret != CL_SUCCESS) {
		printf("clCreateBuffer() failed\n");
		return;
	}
	
	mem[1] = clCreateBuffer(context, CL_MEM_READ_ONLY, 5 * sizeof(int), NULL, &ret);
	if (ret != CL_SUCCESS) {
		printf("clCreateBuffer() failed\n");
		return;
	}

	mem[2] = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 5 * sizeof(int), NULL, &ret);
	if (ret != CL_SUCCESS) {
		printf("clCreateBuffer() failed\n");
		return;
	}

	ret = clEnqueueWriteBuffer(queue, mem[0], CL_TRUE, 0, 5 * sizeof(int), input0, 0, NULL, NULL);
	if (ret != CL_SUCCESS) {
		printf("clEnqueueWriteBuffer() failed\n");
		return;
	}
	
	ret = clEnqueueWriteBuffer(queue, mem[1], CL_TRUE, 0, 5 * sizeof(int), input1, 0, NULL, NULL);
	if (ret != CL_SUCCESS) {
		printf("clEnqueueWriteBuffer() failed\n");
		return;
	}

	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &mem[0]);
	if (ret != CL_SUCCESS) {
		printf("clSetKernelArg() failed\n");
		return;
	}

	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), &mem[1]);
	if (ret != CL_SUCCESS) {
		printf("clSetKernelArg() failed\n");
		return;
	}
	
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), &mem[2]);
	if (ret != CL_SUCCESS) {
		printf("clSetKernelArg() failed\n");
		return;
	}

	size_t global_work_size[1] = {5};
	size_t local_work_size[1] = {1};

	ret = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);
	if (ret != CL_SUCCESS) {
		printf("clEnqueueNDRangeKernel() failed %d\n", ret);
		return;
	}

	clFinish(queue);

	ret = clEnqueueReadBuffer(queue, mem[2], CL_TRUE, 0, 5 * sizeof(int), output, 0, NULL, NULL);
	if (ret != CL_SUCCESS) {
		printf("clEnqueueReadBuffer() failed\n");
		return;
	}
	
    printf("%d %d %d %d %d\n", input0[0], input0[1], input0[2], input0[3], input0[4]);
    printf("%d %d %d %d %d\n", input1[0], input1[1], input1[2], input1[3], input1[4]);
	printf("%d %d %d %d %d\n", output[0], output[1], output[2], output[3], output[4]);
}
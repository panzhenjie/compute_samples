#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ze_api.h>

void main()
{
	zeInit(0);

	uint32_t driverCount = 1;
	ze_driver_handle_t hDriver;
	zeDriverGet(&driverCount, &hDriver);

	uint32_t deviceCount = 1;
	ze_device_handle_t hDevice;
	zeDeviceGet(hDriver, &deviceCount, &hDevice);

	ze_context_desc_t contextDesc = {};
	ze_context_handle_t hContext;
	zeContextCreate(hDriver, &contextDesc, &hContext);
	
	ze_command_queue_desc_t commandQueueDesc = {
		ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC,
		0,
		0,
		0,
		0,
		ZE_COMMAND_QUEUE_MODE_DEFAULT,
		ZE_COMMAND_QUEUE_PRIORITY_NORMAL
	};
	ze_command_queue_handle_t hCommandQueue;
	zeCommandQueueCreate(hContext, hDevice, &commandQueueDesc, &hCommandQueue);
	
	ze_command_list_desc_t commandListDesc = {
		ZE_STRUCTURE_TYPE_COMMAND_LIST_DESC,
		0,
		0,
		0
	};
	ze_command_list_handle_t hCommandList;
	zeCommandListCreate(hContext, hDevice, &commandListDesc, &hCommandList);

	FILE *fd = fopen("kernel_.spv", "r");
	fseek(fd, 0, SEEK_END);
	size_t len = ftell(fd);
	rewind(fd);
	char *spirv = (char *)malloc(len);
	len = fread(spirv, sizeof(char), len, fd);
	fclose(fd);
	
	ze_module_desc_t moduleDesc = {
		ZE_STRUCTURE_TYPE_MODULE_DESC,
		0,
		ZE_MODULE_FORMAT_IL_SPIRV,
		len,
		spirv,
		0,
		0
	};
	ze_module_handle_t hModule;
	zeModuleCreate(hContext, hDevice, &moduleDesc, &hModule, 0);
	
	ze_kernel_desc_t kernelDesc = {
		ZE_STRUCTURE_TYPE_KERNEL_DESC,
		0,
		0,
		"add"
	};
	ze_kernel_handle_t hKernel;
	zeKernelCreate(hModule, &kernelDesc, &hKernel);
	zeKernelSetGroupSize(hKernel, 1, 1, 1);

	int input0_h[5] = {1, 2, 3, 4, 5};
	int input1_h[5] = {6, 7, 8, 9, 10};
	int output_h[5];
	void *input0_d;
	void *input1_d;
	void *output_d;
	
	ze_device_mem_alloc_desc_t allocDesc = {};
	zeMemAllocDevice(hContext, &allocDesc, 5 * sizeof(int), 1024, hDevice, &input0_d);
	zeMemAllocDevice(hContext, &allocDesc, 5 * sizeof(int), 1024, hDevice, &input1_d);
	zeMemAllocDevice(hContext, &allocDesc, 5 * sizeof(int), 1024, hDevice, &output_d);

	zeKernelSetArgumentValue(hKernel, 0, sizeof(void *), &input0_d);
	zeKernelSetArgumentValue(hKernel, 1, sizeof(void *), &input1_d);
	zeKernelSetArgumentValue(hKernel, 2, sizeof(void *), &output_d);
	
	zeCommandListReset(hCommandList);
	zeCommandListAppendMemoryCopy(hCommandList, input0_d, input0_h, 5 * sizeof(int), NULL, 0, NULL);
	zeCommandListClose(hCommandList);
	zeCommandQueueExecuteCommandLists(hCommandQueue, 1, &hCommandList, 0);

	zeCommandListReset(hCommandList);
	zeCommandListAppendMemoryCopy(hCommandList, input1_d, input1_h, 5 * sizeof(int), NULL, 0, NULL);
	zeCommandListClose(hCommandList);
	zeCommandQueueExecuteCommandLists(hCommandQueue, 1, &hCommandList, 0);
	
	ze_group_count_t launchArgs = {5, 1, 1};
	zeCommandListReset(hCommandList);
	zeCommandListAppendLaunchKernel(hCommandList, hKernel, &launchArgs, NULL, 0, NULL);
	zeCommandListClose(hCommandList);
	zeCommandQueueExecuteCommandLists(hCommandQueue, 1, &hCommandList, 0);
	
	zeCommandListReset(hCommandList);
	zeCommandListAppendMemoryCopy(hCommandList, output_h, output_d, 5 * sizeof(int), NULL, 0, NULL);
	zeCommandListClose(hCommandList);
	zeCommandQueueExecuteCommandLists(hCommandQueue, 1, &hCommandList, 0);

	zeCommandQueueSynchronize(hCommandQueue, 0xFFFFFFFF);

	printf("%d %d %d %d %d\n", output_h[0], output_h[1], output_h[2], output_h[3], output_h[4]);
}
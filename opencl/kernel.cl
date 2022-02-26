__kernel void add(__global int *input0, __global int *input1, __global int *output)
{
	int id = get_global_id(0);	
	output[id] = input0[id] + input1[id];
}
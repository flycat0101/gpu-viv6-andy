__kernel void uintAdd2D(__global const uint inputA[4][4], __global const uint inputB[4][4],
							__global uint result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = inputA[i][j] + inputB[i][j];
}

__kernel void uintMult2D(__global const uint inputA[4][4], __global const uint inputB[4][4],
							__global uint result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = inputA[i][j] * inputB[i][j];
}

__kernel void uintCopy2D(__global const uint input[4][4], __global uint result[4][4],
							const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = input[i][j];
}

__kernel void uintDivide2D(__global const uint inputA[4][4], __global const uint inputB[4][4],
							__global uint result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = (uint)divide((float)inputA[i][j],(float)inputB[i][j]);
}

__kernel void uintClz2D(__global const uint input[4][4], __global uint result[4][4],
							const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = (uint)clz(input[i][j]);
}
__kernel void intAdd2D(__global const int inputA[4][4], __global const int inputB[4][4],
							__global int result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = inputA[i][j] + inputB[i][j];
}

__kernel void intMult2D(__global const int inputA[4][4], __global const int inputB[4][4],
							__global int result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = inputA[i][j] * inputB[i][j];
}

__kernel void intCopy2D(__global const int input[4][4], __global int result[4][4],
							const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = input[i][j];
}

__kernel void intDivide2D(__global const int inputA[4][4], __global const int inputB[4][4],
							__global int result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = (int)divide((float)inputA[i][j],(float)inputB[i][j]);
}

__kernel void intClz2D(__global const int input[4][4], __global int result[4][4],
							const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = (int)clz(input[i][j]);
}
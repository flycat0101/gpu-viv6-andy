__kernel void floatAdd2D(__global const float inputA[4][4], __global const float inputB[4][4],
							__global float result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = inputA[i][j] + inputB[i][j];
}

__kernel void floatMult2D(__global const float inputA[4][4], __global const float inputB[4][4],
							__global float result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = inputA[i][j] * inputB[i][j];
}

__kernel void floatCopy2D(__global const float input[4][4], __global float result[4][4],
							const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = input[i][j];
}

__kernel void floatDivide2D(__global const float inputA[4][4], __global const float inputB[4][4],
							__global float result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = divide(inputA[i][j],inputB[i][j]);
}

__kernel void floatClz2D(__global const float input[4][4], __global float result[4][4],
							const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = (float)clz((int)input[i][j]);
}
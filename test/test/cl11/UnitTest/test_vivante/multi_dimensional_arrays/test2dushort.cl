__kernel void ushortAdd2D(__global const ushort inputA[4][4], __global const ushort inputB[4][4],
							__global ushort result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = inputA[i][j] + inputB[i][j];
}

__kernel void ushortMult2D(__global const ushort inputA[4][4], __global const ushort inputB[4][4],
							__global ushort result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = inputA[i][j] * inputB[i][j];
}

__kernel void ushortCopy2D(__global const ushort input[4][4], __global ushort result[4][4],
							const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = input[i][j];
}

__kernel void ushortDivide2D(__global const ushort inputA[4][4], __global const ushort inputB[4][4],
							__global ushort result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = (ushort)divide((float)inputA[i][j],(float)inputB[i][j]);
}

__kernel void ushortClz2D(__global const ushort input[4][4], __global ushort result[4][4],
							const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = (ushort)clz(input[i][j]);
}
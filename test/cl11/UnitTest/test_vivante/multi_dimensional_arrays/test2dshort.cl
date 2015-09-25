__kernel void shortAdd2D(__global const short inputA[4][4], __global const short inputB[4][4],
							__global short result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = inputA[i][j] + inputB[i][j];
}

__kernel void shortMult2D(__global const short inputA[4][4], __global const short inputB[4][4],
							__global short result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = inputA[i][j] * inputB[i][j];
}

__kernel void shortCopy2D(__global const short input[4][4], __global short result[4][4],
							const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = input[i][j];
}

__kernel void shortDivide2D(__global const short inputA[4][4], __global const short inputB[4][4],
							__global short result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = (short)divide((float)inputA[i][j],(float)inputB[i][j]);
}

__kernel void shortClz2D(__global const short input[4][4], __global short result[4][4],
							const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = (short)clz(input[i][j]);
}
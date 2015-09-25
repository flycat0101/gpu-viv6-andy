__kernel void charAdd2D(__global const char inputA[4][4], __global const char inputB[4][4],
							__global char result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = inputA[i][j] + inputB[i][j];
}

__kernel void charMult2D(__global const char inputA[4][4], __global const char inputB[4][4],
							__global char result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = inputA[i][j] * inputB[i][j];
}

__kernel void charCopy2D(__global const char input[4][4], __global char result[4][4],
							const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = input[i][j];
}

__kernel void charDivide2D(__global const char inputA[4][4], __global const char inputB[4][4],
							__global char result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = (char)divide((float)inputA[i][j],(float)inputB[i][j]);
}

__kernel void charClz2D(__global const char input[4][4], __global char result[4][4],
							const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = (char)clz(input[i][j]);
}
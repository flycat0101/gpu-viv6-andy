__kernel void ucharAdd2D(__global const uchar inputA[4][4], __global const uchar inputB[4][4],
							__global uchar result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = inputA[i][j] + inputB[i][j];
}

__kernel void ucharMult2D(__global const uchar inputA[4][4], __global const uchar inputB[4][4],
							__global uchar result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = inputA[i][j] * inputB[i][j];
}

__kernel void ucharCopy2D(__global const uchar input[4][4], __global uchar result[4][4],
							const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = input[i][j];
}

__kernel void ucharDivide2D(__global const uchar inputA[4][4], __global const uchar inputB[4][4],
							__global uchar result[4][4], const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = (uchar)divide((float)inputA[i][j],(float)inputB[i][j]);
}

__kernel void ucharClz2D(__global const uchar input[4][4], __global uchar result[4][4],
							const int sizeX, const int sizeY) {
	const int i = get_global_id(0);
	const int j = get_global_id(1);
	if (i < sizeX && j < sizeY)
		result[i][j] = (uchar)clz(input[i][j]);
}
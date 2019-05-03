there are five tutorial case, and each are help customer to use the OpenVx framework step by step.

vx_tutorial1:
The simplest case based on OpenVX framework, which use test_gray.bmp as input image and test_sobel.bmp as output image by default.
this case using only one node in graph, which processing sobel3x3 algorithm on input image.
NO arguments are required to run this program.

vx_tutorial2:
This case is very similar to the vx_tutorial1, the only difference is that more than one node(five node) are added in the graph.
The five node are Gaussian3x3 -> Sobel3x3 -> Magniude -> ConvertDepth -> Threshold .
The input image will be processed in accordance with this algorithm sequence.
NO arguments are required to run this program.

vx_tutorial3:
vx_tutorial1 and vx_tutorial2 only using build-in nodes in OpenVX library, but in vx_tutorial3 shows how to use usr-define node in application based on OpenVX framework.
This tutorial using two image as inputs, by default named left_gray.bmp and right_gray.bmp. And defined the kernel named vxcAbsDiff which calc the absolute different value between two input images and write output image.
In this kernel, shows how to use EVIS instructions to accelerate , and also shows how to using read and write image instructions.
The relevant validate functions are also introduced, shows how to verify the kernel parameters.
NO arguments are required to run this program.

vx_tutorial4:
This tutorial include two projects: kernel library project in the folder of kernel and application project which in app folder. Appliation project using kernel library.
The tutorial4 kernel implements the Bayer Demosaic Filtering, please refer to the relevant algorithm documents at the path of \vx_tutorial4\app\Doc.
In this kernel, we are not only using EVIS instructions, but also using DP tools to define the specific instructions, we can get DP uniforms in the folder of unifom.
In application project,
	The parameter set of vx_tutorial4 is as the following:
	-f  data_file_list.txt    Input data list. Please put your test data names here (data at per row, ).
	-w  1920           	Width of input data
	-h  1080            Height of input data
	-c  10              GPU clock on FPGA

vx_tutorial5:
This tutorial is same with vx_tutorial3, using the same algorithm. The difference between this two tutorials are:
1. vx_tutorial5 using the vx_absdiff.vx file to define kernel , while in vx_tutorial3, the kernel is contained in main.c file using string type.
2. vx_tutorial3 using on-line kernel build method. and in vx_tutorial5, using Binary Source method.
In addition to the above two differences, the whole two tutorials are exactly the same.

 
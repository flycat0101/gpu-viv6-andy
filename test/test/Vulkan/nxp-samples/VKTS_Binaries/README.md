VulKan ToolS (VKTS) Binaries:
-----------------------------

This folder and sub folders contains all executables, shaders and assets. The examples are copied  
into this folder after the build. Generated mip maps, cube maps and BSDF look up textures are stored in the `cache` folder.
By default, this content is already generated.
  
__This folder has to be the working directory, otherwise shaders and assets are not found__.  
  
Using Android, the needed asset files will be copied in the asset folder after executing `python build_project.py`.

Inside `shader/GLSL/` there is a python build script for the host system, which do compile all available GLSL source files.  
The resulting SPIR-V binaries are deployed into `shader/SPIR/V`.  

To enable the standard set of Vulkan validation layers, set the following environment variables:
  
Linux:  
`export VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_standard_validation`  
  
Windows:  
`set VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_standard_validation`  
  
  
Used libraries:
---------------

- [LunarG Vulkan SDK](http://vulkan.lunarg.com) (see [License](/VKTS/LunarG_license.html))
  
  
Used applications:
------------------

- [Bitmap Font Generator](http://www.angelcode.com/products/bmfont/)
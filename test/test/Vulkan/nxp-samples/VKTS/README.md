VulKan ToolS (VKTS):
--------------------

Authors:

**Norbert Nopper** (norbert@nopper.tv)

VKTS version: **0.9.2**

Depending Vulkan API header version: **1.0.21**

Depending Vulkan extensions, when using a window system:

- `VK_KHR_swapchain`
- `VK_KHR_surface`

Linux X11:
- `VK_KHR_xlib_surface` or 
- `VK_KHR_xcb_surface`  

Linux i.MX8:
- `VK_KHR_display`
  
Android:
- `VK_KHR_android_surface`
    
Windows:
- `VK_KHR_win32_surface`  


About:
------

VulKan ToolS (VKTS) is a cross platform, C++ 11 helper library for [Vulkan](https://www.khronos.org/vulkan) released under the [MIT License](VKTS_license.txt).


Used libraries (beside C++ 11):
-------------------------------

- [LunarG Vulkan SDK](http://vulkan.lunarg.com)
- [GLM](http://glm.g-truc.net)


Supported platforms:
--------------------

- Linux X11, i.MX8 or console only (GNU GCC)
- Android (Clang) 
- Microsoft Windows or console only (GNU MinGW & Visual C++ 2015)


Features:
---------

- Platform abstraction.
- Logging functions.
- Profiling functions.
- Matrix, plane, sphere and quaternion classes and functions.
- Vulkan wrapper objects and factory functions.
- Basic multi threaded engine including synchronization, time and CPU querying.   
- Basic native display handling including multi display support.
- Basic native window handling including fullscreen support.
- Basic input handling: Gamepad, keyboard and mouse.
- None visual configuration for console/compute only applications.
- Loading and saving of text, binary and image files (TGA and HDR).
- VKTS scene file format:  
    * 3D objects (incl. node hierarchy)  
    * Meshes (incl. sub meshes)  
    * Materials (Phong and BSDF)  
    * Textures (TGA and HDR)  
    * Animations (incl. node and armature) 
- Blender Python exporter for the VKTS scene file format.
- Input controller for cameras and 3D objects. 
- Bitmap font rendering.
- GLSL to SPIR-V build script.
- Interpolator and converter for constant, linear and bezier key frame data.


How to build:
-------------

- [System requirements and step by step tutorial](/VKTS_Documentation/BUILD.md)


Changelog:
----------

08/10/2016
- Continued on particle system export and loading.

08/09/2016
- Added layers to node tree.
- Blender export script writes now the layers.

07/29/2016
- Updated documentation.

07/27/2016
- Fixed cycles material exporter: Translate mapping now works.
- Real-time PBR working.

07/19/2016
- Fixed CMake files for i.MX8.
- Added possibility to choose physical device via console option (e.g. "-pd 1" for physical device at index 1).

07/18/2016
- Fixed VK_KHR_display configuration.
- Added support for i.MX8 device.

07/16/2016
- Added several, minor features. Please have a look at the git history.

06/28/2016
- Fixed, working and tested cube map generation.

06/15/2016
- Added raw data file and load functions.
- Added more render functions to later pre-filter images.

06/05/2016
- Added random functions.

06/04/2016
- Added cube map image generation and creation.

05/29/2016
- Added feature to save mip level and array layer of an image data.
- Added feature to merge image data for both mip maps levels and array layers.

05/28/2016
- Added layers for image data.
- Added sampling for image data.

05/26/2016
- Mip maps of images are now only created for textures using them.

05/25/2016
- Made resize code more robust and reliable.
- Fixed profile code for POSIX.

05/24/2016
- Fixed/improved directory creation.
- Improved font class.
- Finalized profiling.

05/23/2016
- Improved performance in binary buffer and added better creation checks.
- Fixed context class.
- Added cpu load and ram usage query per application.

05/21/2016
- Added disable/enable of cache.
- Added feature to poll for finished tasks.

05/20/2016
- Fixed/improved image barrier.
- Removed warning in vector template.
- Added CPU and FPS profiling.
- Updated to LunarG SDK 1.0.13.0.

05/19/2016
- All scripts are now pyhton scripts.
- Fixed/improved Android exit and shutdown code.
- Continued on BSDF material loader.

05/18/2016
- Added OS independent directory creation.
- Added cache for generated assets.
- Refactored bindings.
- Continued on BSDF material.
- Update to latest Android NDK.

05/17/2016
- Optimized final layout for images.

05/14/2016
- Fixed/improved copy image, buffer and the used barriers.

05/13/2016
- Working and tested font class.

05/12/2016
- Finalized font creation class, not tested.

05/10/2016
- Fixed event creation.

05/09/2016
- Refactored/fixed/improved moveable class.
- Added dot product and unary operators for quaternions.
- Removed validation layers warnings.

05/08/2016
- Finalized default graphics pipeline class and added default compute pipeline class.

05/07/2016
- Added queue for executed tasks and removed old interface.
- Added default graphics pipeline class.

05/06/2016
- Added object limit for scene graph.
- Made own vector, list and map classes more robust.

05/05/2016
- Updated to LunarG SDK 1.0.11.1.

05/04/2016
- Added missing cstlib header file.
- Default display detection is more relaxed under Linux now.
- Continued on font class. Add font shaders.
- Improved input controller and updating its moveable.

05/03/2016
- Added font class.

04/30/2016
- Enabled VK_KHR_xlib_surface, as now present in the VulkanSDK 1.0.11.0 again.

04/28/2016
- Updated to LunarG SDK 1.0.11.0.

04/23/2016
- Disabled VK_KHR_xlib_surface temporarily, as currently not present in the VulkanSDK.

04/21/2016
- Added binding for shadow mapping.

04/20/2016
- Refactored shader bindings.

04/14/2016
- Updated documentaion structure and clarified topics.  
- Optimized Android file loading.
- Added automatic gathering of base directory.
- Added invalidate and flush for mapped memory. 

04/13/2016
- Fixed code for target forward calculation in IMoveable.  
- Fixed bug in image layout transitions.  

04/12/2016
- Fixed bug, that transform matrix from scene object was set non dirty too early.  
- Scene object is now inherited from IMoveable.  
- Extended IMoveable.  

04/11/2016
- Added BSDF material loader.  
- Added node visitor function to recursivly set parameters.  
- Continued on Blender Cycles exporter.  

04/10/2016
- Continued on BSDF shader class.  

04/09/2016
- Split up textures into device images, having sampler and view.  
- Optimized material management, by avoiding unnecessary clone.  

04/07/2016
- Refactored overwrite visitor to be more flexible.  
- Fixed frustum culling.  
- Fixed plane class.  
- Fixed bounding sphere calcualtion.  
- Updated to LunarG SDK 1.8.0.  

04/06/2016
- Fixed camera creation.  

04/05/2016
- Fixed bug in Blender export script.  
- Increased max joints to 96. Added error output, if too many joints are used.  

04/04/2016
- Added physical device features query.  
- Added blend overwrite.  

04/01/2016
- Fixed camera position creation.  
- Fixed animation duration in Blender exporter.  
- Improved Blender Cycles material exporter.  

03/30/2016
- Refactored code for having BSDF shader.  
- Added first version of Blender Cycles material exporter.  

03/29/2016
- Fixed issue, that mesh was rendered once when shared with other nodes.  

03/28/2016
- Multiple graphics pipelines can be past to the scene, which makes it more robust and flexible.  

03/27/2016
- Fixed bezier interpolator regarding infinite loop.  

03/26/2016
- Added wrapper class for compute pipeline.  
- Improved file saving on Android, as file does not have to exist.  
- Added possibility to overwrite scene rendering behaviour.  
- Added to scene file, if transparency is enabled or not.  

03/25/2016
- Fixes, that VKTS can be build without any windowing system.  
- Enhanced fence wrapper class.  

03/24/2016
- Added linux mouse handler for devices using VK_KHR_display extension.  

03/23/2016
- Added linux keyboard handler for devices using VK_KHR_display extension.  

03/22/2016
- Improved Android file access wrapper classes.  
- Using VK_PRESENT_MODE_FIFO_KHR for present mode as default.   

03/21/2016
- Removed legacy defines.  
- Using Vulkan prototypes for extensions.  
- Added 32 bit CMake build configurations.  
- Added initial support for VK_KHR_display extension.  

03/19/2016
- Improved wrapper class for initial resources.  

03/17/2016
- Added view frustum culling.  
- Scene file allows optional definition of displacement.  
- Scene file allows optional definition of AABB.  

03/16/2016
- Continued on aabb and sphere class.  

03/15/2016
- Continued on classes for culling and intersecting test.  
- Using current extent of surface capabilities for swapchain creation.  

03/14/2016
- Added frustum class for doing frustum culling.  
- Added first version of oriented and axis aligned bounding boxes.  
- Add changes for Android NDK r11, which support Vulkan:  
    * Using recommended clang now. Removed detected warnings by clang.  
    * Using platform android-24.  
           
03/12/2016
- Updated to Vulkan API header version 1.0.5 and using LunarG Vulkan SDK 1.0.5.0.  

03/11/2016
- Added general extension handler.  
- Added debug reporting.  

03/10/2016
- Added plane and sphere class for distance calculations.  

03/09/2016
- Switched to XInput version 9.1.0 on Windows.  

03/08/2016
- Improved/fixed task queue by limiting elements and avoiding "infinite" heap grow.  
- Improved scenegraph for parallel execution.  

03/05/2016
- Improved/fixed Linux gamepad detection code.  

03/04/2016
- Fixed swap chain creation, that actual number of images is used.   
- Improved smart pointer list, map and vector.  

03/03/2016
- Made command buffers resetable.  

03/02/2016
- Fixed keyboard lookup table, which did write out of bounds.  

02/28/2016
- Added layout qualifier to all shaders.  
- Fixed projection matrices, that z goes from 0 to 1.  
- Removed invalid access flags.  

02/24/2016
- Retested for AMD, Intel and NVIDIA GPUs and removed all layer warnings.  
- Fixed task infinite wait in case execution thread was faster.  
- Improved scene class, that only used resources can be bound.  

02/23/2016
- Fixed and improved the task executor engine.  

02/21/2016
- Updated to GLM 0.9.7.3.  
- Extended file load and save functionality for Android devices.  

02/19/2016
- Refactored physical device wrapper class and added new query function.  
- Under Linux, the VK_KHR_xcb_surface extension is now preferred.  

02/16/2016
- First public release.  

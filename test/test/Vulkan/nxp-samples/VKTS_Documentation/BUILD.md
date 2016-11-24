How to build the VulKan ToolS (VKTS) examples:
----------------------------------------------

General requirements:
---------------------
  
- [LunarG Vulkan SDK](http://vulkan.lunarg.com) version has to be 1.0.21.1.
- Vulkan headers and libraries have to be visible by the used compiler and/or IDE.
- [Python](https://www.python.org/) version has to be 3.5 or higher.  


Following requirements do depend on the platform:

![Android](images/android_logo.png) Android:
---

- Building: ndk-build  
    - Python build scrips included.  
    - [Android NDK](http://developer.android.com/ndk/index.html) with Vulkan support required.  
    - [Apache Ant](https://ant.apache.org/) required.  
      

![Linux](images/linux_logo.png) Linux:
---

- Building: Eclipse IDE for C/C++ Developers (recommended)  
    - [Eclipse IDE for C/C++ Developers](https://www.eclipse.org/downloads/) project files included.   
- Building: make  
    - Minimum [CMake](https://cmake.org/) version has to be 3.2.0.  
- Needed libraries: libx11-xcb-dev libxrandr-dev  


![Windows](images/windows_logo.png) Windows:
---

- Building: Visual C++ (recommended)  
    - Minimum [CMake](https://cmake.org/) version has to be 3.2.0.  
    - [Visual Studio 2015](https://www.visualstudio.com/en-us/downloads/visual-studio-2015-downloads-vs.aspx) or later is required.  
- Building: Eclipse IDE for C/C++ Developers  
    - [Eclipse IDE for C/C++ Developers](https://www.eclipse.org/downloads/) project files included.  
- Building: make  
    - Minimum CMake version has to be 3.2.0.  
    - [MinGW-w64 - for 32 and 64 bit Windows](https://sourceforge.net/projects/mingw-w64/) required.  


---

Using Eclipse:
--------------

1. Install [Eclipse IDE for C/C++ Developers](https://www.eclipse.org/downloads/) for Linux or Windows.
2. Install GCC for Linux or [MinGW-w64 - for 32 and 64 bit Windows](https://sourceforge.net/projects/mingw-w64/) for Windows.
3. Import all folders as an existing project in Eclipse. 
4. Set the build configuration of VKTS and the VKTS_Example* in Eclipse to your operating system.
5. __Build VKTS first__.
6. Build VKTS_Example*.
7. The executables are located in the [VKTS_Binaries](VKTS_Binaries/README.md) folder.
8. __The VKTS_Binaries folder has to be the working directory, otherwise shaders and assets are not found__.


---

Using Visual C++ 2015:
----------------------

1. Install [Visual Studio 2015 Community Edition](https://www.visualstudio.com/en-us/downloads/visual-studio-2015-downloads-vs.aspx) for Windows.
2. Run CMake and configure for "Visual Studio 14 2015 Win64".
   Add `-DVKTS_WSI=VKTS_NO_VISUAL` to create without windowing system.
3. Set the source code directory to VKTS.
4. Set the build directory to VKTS/MSVC (Note: This build directory is expected by the examples).
5. Open the Visual C++ solution file and __build VKTS first__.
6. Do step 3 to 5 with VKTS_Example*.
7. The executables are located in the [VKTS_Binaries](VKTS_Binaries/README.md) folder.
8. __The VKTS_Binaries folder has to be the working directory, otherwise shaders and assets are not found__.


---

Using make:
-----------

1. Install a GNU GCC toolchain.
2. Run CMake and configure "MSYS Makefiles" for Windows or "Unix Makefiles" for Linux.
   Add `-DVKTS_WSI=VKTS_NO_VISUAL` to create without windowing system.
3. Set the source code directory to VKTS.
4. Set the build directory to VKTS (Note: This build directory is expected by the examples). 
5. __Run make and build VKTS first__.
6. Do step 3 to 5 with VKTS_Example*.
7. The executables are located in the [VKTS_Binaries](VKTS_Binaries/README.md) folder.


---

Using LunarG SDK under Linux:
-----------------------------

Append the following lines to your `.profile` file in your home directory: 

`#`
`# Vulkan development setup.`
`#`

`export VULKAN_SDK=~/Development/VulkanSDK/1.0.21.1/x86_64`

`export PATH=$PATH:$VULKAN_SDK/bin`
`export LD_LIBRARY_PATH=$VULKAN_SDK/lib`
`export VK_LAYER_PATH=$VULKAN_SDK/etc/explicit_layer.d`

`C_INCLUDE_PATH=$VULKAN_SDK/include` 
`export C_INCLUDE_PATH`
`CPLUS_INCLUDE_PATH=$VULKAN_SDK/include` 
`export CPLUS_INCLUDE_PATH`
`LIBRARY_PATH=$VULKAN_SDK/lib`
`export LIBRARY_PATH`

Finally, change the path of `VULKAN_SDK` depending on your installation directory and SDK version.


---

Installing Intel Linux Vulkan Driver:
-------------------------------------
  
Execute the following command to add the repository with the Intel Vulkan driver:  
  
`sudo apt-add-repository ppa:canonical-x/vulkan`  
`sudo apt update`  
  
Execute the following command to install the driver:  
  
`sudo apt install mesa-vulkan-drivers`  
  
After this, create the following file if it does not exist:  
  
`/etc/X11/xorg.conf`  
  
Append/insert following lines to the above file to enable DRI3:  

`Section "Device"`
`    Identifier  "Intel Graphics"`
`    Driver      "intel"`
`    Option      "SwapBuffersWait" "0"`
`    Option      "DRI" "3"`
`EndSection`


---

Using Android NDK:
------------------

The needed shaders, textures and scene files are automatically copied into the assets folder by
executing `python build_project.py`. After the build, the assets are packed in the apk file. 

Note:
With the current beta NDK, copy the `include/vulkan` and `lib/libvulkan.so` from `android-24` to `android-23`,
as `android-23` is still used.

The following command builds the VKTS library:

1. Open console in `Android/jni` project folder and execute following command:  
    - `ndk-build`


The following command creates the Android project:

1. Open console in `Android` project folder and execute following command:  
    - `python create_project.py`

The following command copies the needed assets and builds the Android project:

2. Open console in `Android` project folder and execute following command:  
    - `python build_project.py`

The following command installs the Android project:

3. Open console in `Android` project folder and execute following command:  
    - `python install_project.py`

The following commands allows to display the log output:

4. Open another console execute following command in the `VKTS_Binaries` folder:  
    - `python android_log.py` 


Setting up the Android device:
------------------------------

The following has only be done once:

1. Enable developer mode on the Android device.
2. Enable USB debugging on the Android device.

The following has only be done once, if automatic TCPIP debugging is wanted:

1. Enable the wireless network on the Android device.  
2. Execute the following commands without the comments:  
    - `adb root`                         // Restart adb as root  
    - `adb remount`                      // Remount /system for read and write  
    - `adb pull /system/build.prop`      // Get the provided file  
3. Append 'service.adb.tcp.port=5555' in downloaded build.prop to autostart adbd in TCPIP mode  
4. Execute the following command without the comment:  
    - `adb push build.prop /system`      // Store the modified file  


Common adb commands:
--------------------

- List Android devices:  
    - `adb devices`  
- Restart adbd daemon with usb:  
    - `adb usb`  
- Restart adbd daemon with tcpip and given port:  
    - `adb tcpip 5555`  
- Connect to a TCPIP enabled device:  
    - `adb connect 192.168.0.12:5555`    // Change the IP address  

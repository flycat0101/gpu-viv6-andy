import glob
import os
import shutil
import subprocess  

####################
#
# Functions
#
####################

def copy(src, dst):
    # Create directory if needed
    if not os.path.exists(dst):
        os.makedirs(dst)

    # Copy exact file or given by wildcard 
    for filename in glob.glob(src):
        print("Copying asset '%s'" % (os.path.basename(filename)))
        shutil.copy(filename, dst)        
        
####################
#
# Main
#
####################

print("Copying project assets")

copy("../../VKTS_Binaries/shader/SPIR/V/phong_skinning.vert.spv", "./assets/shader/SPIR/V/")
copy("../../VKTS_Binaries/shader/SPIR/V/phong_skinning.frag.spv", "./assets/shader/SPIR/V/")
copy("../../VKTS_Binaries/astro_boy/*.vkts", "./assets/astro_boy/")
copy("../../VKTS_Binaries/astro_boy/*.tga", "./assets/astro_boy/")

print("Building project")

os.chdir("jni")

subprocess.call("ndk-build", shell=True)

os.chdir("..")
        
subprocess.call("ant debug", shell=True)  

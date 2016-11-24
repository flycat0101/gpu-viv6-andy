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

copy("../../VKTS_Binaries/shader/SPIR/V/phong.vert.spv", "./assets/shader/SPIR/V/")
copy("../../VKTS_Binaries/shader/SPIR/V/phong.frag.spv", "./assets/shader/SPIR/V/")
copy("../../VKTS_Binaries/cartoon_tree_with_plane/*.vkts", "./assets/cartoon_tree_with_plane/")
copy("../../VKTS_Binaries/cartoon_tree_with_plane/*.tga", "./assets/cartoon_tree_with_plane/")

print("Building project")

os.chdir("jni")

subprocess.call("ndk-build", shell=True)

os.chdir("..")
        
subprocess.call("ant debug", shell=True)  

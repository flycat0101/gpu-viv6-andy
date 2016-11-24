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

copy("../../VKTS_Binaries/test/general/*.txt", "./assets/test/general/")
copy("../../VKTS_Binaries/test/general/*.tga", "./assets/test/general/")
copy("../../VKTS_Binaries/test/general/*.hdr", "./assets/test/general/")

print("Building project")

os.chdir("jni")

subprocess.call("ndk-build", shell=True)

os.chdir("..")
        
subprocess.call("ant debug", shell=True)  

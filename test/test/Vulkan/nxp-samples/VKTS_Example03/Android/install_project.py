import os
import subprocess  

####################
#
# Functions
#
####################
        
####################
#
# Main
#
####################

print("Installing project")
        
subprocess.call("adb install -r bin/NativeActivity-debug.apk", shell=True)  

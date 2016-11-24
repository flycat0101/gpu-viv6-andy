import os
import signal
import subprocess  
import sys

####################
#
# Functions
#
####################

def catchSignal(signal, frame):
    print("")
    print("Logging stopped")
    sys.exit(0)
        
####################
#
# Main
#
####################

signal.signal(signal.SIGINT, catchSignal)

print("Cleaning log file and starting logging (CTRL-C to stop)")
        
subprocess.call("adb logcat -c", shell=True)  
subprocess.call("adb logcat -s VKTS", shell=True)

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

glslExtensions =  [".vert", ".tesc", ".tese", ".geom", ".frag", ".comp"]

for glslFilename in os.listdir("."):
    
    currentExtension = os.path.splitext(glslFilename)[1]
    
    if currentExtension not in glslExtensions:
        continue
    
    spirFilename = glslFilename + ".spv"
    
    print("Building '%s'" % (glslFilename))
    
    #
    
    p = subprocess.Popen(["glslangValidator", "-V", glslFilename, "-o", spirFilename], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output, error = p.communicate()
    
    splitOutput = output.decode().split("\n")

    for currentOutput in splitOutput:
            
        currentOutput = currentOutput.strip()
        
        if currentOutput[:5] == "ERROR" or currentOutput[:7] == "WARNING":
            print("%s" % (currentOutput))
    
    print("")        

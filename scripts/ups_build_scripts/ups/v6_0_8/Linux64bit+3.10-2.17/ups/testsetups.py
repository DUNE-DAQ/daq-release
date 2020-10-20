
import sys
sys.path.insert(0,"/scratch/mengel/p")
import setups
import os
 
print "here1"
ups = setups.setups()
print "here2"
ups.use_python("v2_6_2")
print "here3"
ups.setup("fife_utils")
print "FIFE_UTILS_DIR is ", os.environ["FIFE_UTILS_DIR"]
print "PYTHON_DIR is ", os.environ["PYTHON_DIR"]
print ups.ups("list","upd")

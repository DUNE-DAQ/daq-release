#!/usr/bin/env python
import os
import re
import sys
import string

# os.popen3 becomes deprecated, replace with newer subprocess module if available
try:
    from subprocess import Popen, PIPE
    POPEN_AVAILABLE = 1
except:
    POPEN_AVAILABLE = 0
    

DEFAULT_PYTHON_VERSION = ''
DEFAULT_SETUPS_DIR = os.environ.get('SETUPS_DIR', '/usr/local/etc')

#############################################################
# User interface:
#  import ups
#  optional:
#       ups.getUps([setupsDir=/path/to/setups.sh])
#  
#  ups.setup('my project spec' [, setupsDir=/path/to/setups.sh])
#############################################################
# returns the address of a singleton UpsManager object:

singletonUps = None
def getUps(setupsDir=DEFAULT_SETUPS_DIR):
    global singletonUps
    if (singletonUps is None):
        singletonUps = UpsManager(setupsDir)
    return singletonUps

def setup(arg, setupsDir=DEFAULT_SETUPS_DIR):
    upsmgr = getUps(setupsDir)
    upsmgr.setup(arg)

def unsetup(arg, setupsDir=DEFAULT_SETUPS_DIR):
    upsmgr = getUps(setupsDir)
    upsmgr.unsetup(arg)

#############################################################

def set_setupsDir(setupsDir=DEFAULT_SETUPS_DIR):
    global DEFAULT_SETUPS_DIR
    DEFAULT_SETUPS_DIR = setupsDir
#############################################################

# force a reload and exec of the requested version of python:
def use_python(pythonVersion=DEFAULT_PYTHON_VERSION):
    # did we setup python at all?  Or are we getting it by default from the path?
    alreadySetup = os.environ.get('SETUP_PYTHON') is not None
    
    # are we already using the requested version?
    alreadyUsing = alreadySetup and re.search('python %s' % pythonVersion, os.environ.get('SETUP_PYTHON')) is not None
    
    if not alreadyUsing:
        # get a ups instance:
        ups = getUps()

        # unsetup if we are already setup:
        if alreadySetup: ups.unsetup('python')

        # now set it up
        ups.setup('python %s' % pythonVersion)

        # now exec into the context of the requested version of python:
        # were we running the python interpreter itself?  Special handling required!
        if (sys.argv == ['']):
            sys.argv = [os.environ.get('PYTHON_DIR', '') + '/bin/python']
        else:
            sys.argv.insert(0, os.environ.get('PYTHON_DIR', '') + '/bin/python')

        # bye bye, exec into another context:
        pythonDir = os.path.normpath(os.environ.get('PYTHON_DIR'))
        os.execve('%s/bin/python' % pythonDir, sys.argv, os.environ)

##############################################################################

class upsException(Exception):
    def __init__(self, msg):
        self.args = msg
    
class UpsManager:
    def __init__(self, setupsDir=DEFAULT_SETUPS_DIR):

        # initial setup of ups itself:
        os.environ['UPS_SHELL'] = 'sh'

        # initial setup of ups itself:
        os.environ['UPS_SHELL'] = 'sh'
        if POPEN_AVAILABLE:
            f = Popen('. %s/setups.sh; ' % setupsDir + \
                      'echo os.environ\\[\\"UPS_DIR\\"\\]=\\"${UPS_DIR}\\"; ' + \
                      'echo os.environ\\[\\"PRODUCTS\\"\\]=\\"${PRODUCTS}\\";' + \
                      'echo os.environ\\[\\"SETUP_UPS\\"\\]=\\"${SETUP_UPS}\\";' + \
                      'echo os.environ\\[\\"PYTHONPATH\\"\\]=\\"${PYTHONPATH}\\";',
                      shell=True,
                      stdout=PIPE).stdout
        else:
            f = os.popen('. %s/setups.sh; ' % setupsDir + \
                         'echo os.environ\\[\\"UPS_DIR\\"\\]=\\"${UPS_DIR}\\"; ' + \
                         'echo os.environ\\[\\"PRODUCTS\\"\\]=\\"${PRODUCTS}\\";' + \
                         'echo os.environ\\[\\"SETUP_UPS\\"\\]=\\"${SETUP_UPS}\\";' + \
                         'echo os.environ\\[\\"PYTHONPATH\\"\\]=\\"${PYTHONPATH}\\";')
        exec f.read()
        f.close()

        # we need to initialize the following so that we can
        #  make the correct changes to sys.path later when products
        #  we setup modify PYTHONPATH
        self._pythonPath = os.environ.get('PYTHONPATH', '')
        self._sysPath = sys.path
        (self._internalSysPathPrepend, self._internalSysPathAppend) = self._getInitialSyspathElements()

    ############################################################################
    # set a product up:
    def setup(self, arg):
        try:
            self._inhaleresults("%s/bin/ups setup %s" % (os.environ.get('UPS_DIR'), arg))
            self._updateImportPath()
        except upsException:
            raise

    ############################################################################
    # unsetup a product:
    def unsetup(self, arg):
        try:
            self._inhaleresults("%s/bin/ups unsetup %s" % (os.environ.get('UPS_DIR'), arg))
            self._updateImportPath()
        except upsException:
            raise

    ############################################################################ 
    # PRIVATE METHODS BELOW THIS POINT.
    #
    def _getInitialSyspathElements(self):
        pyPath = string.split(self._pythonPath, ':')
        sysPath = self._sysPath

        # sysPath always includes '' at the front of the list
        #  (current working directory)
        internalSysPathPrepend = ['',]
        sysPath = sysPath[1:]

        # now, what else is in sysPath that is NOT in PYTHONPATH
        # (and hence must have come from the internals)?
        internalSysPathAppend = []
        for element in sysPath:
            if (element not in pyPath):
                internalSysPathAppend.append(element)
        return (internalSysPathPrepend, internalSysPathAppend)
        
    ############################################################################
    def _setPythonPath(self):
        self._pythonPath = os.environ.get('PYTHONPATH', '')

    ############################################################################
    def _updateImportPath(self):
        # make sure that sys.path reflects what is in PYTHONPATH after
        # any setups are performed!
        if (os.environ.get('PYTHONPATH', '') != self._pythonPath):
            pypathElements = string.split(os.environ.get('PYTHONPATH', ''), ':')
            # sys.path includes the current working directory as the first element:
            sys.path = self._internalSysPathPrepend
            # now append each pythonpath element
            for element in pypathElements:
                sys.path.append(element)
            # now append whatever was 'internal' originally
            sys.path = sys.path + self._internalSysPathAppend
        # update our reference to PYTHONPATH
        self._setPythonPath()

    ############################################################################
    def _inhaleresults(self, cmd):
        if POPEN_AVAILABLE:
            p = Popen(cmd, shell=True, 
                      stdin=PIPE, stdout=PIPE, stderr=PIPE, close_fds=True)
            (stdin, stdout, stderr) = (p.stdin, p.stdout, p.stderr)
        else:
            (stdin,stdout,stderr) = os.popen3(cmd)
        try:
            filename = stdout.read()
            filename = filename[0:-1]
            if (filename == "/dev/null"):
                msg = stderr.read()
                raise upsException(msg)
        finally:
            stdin.close()
            stdout.close()
            stderr.close()

        cutHere = '--------------cut here-------------'
	setup = open(filename, 'a')
	setup.write(self._getNewPythonEnv(cutHere))
	setup.close()

        if POPEN_AVAILABLE:
            f = Popen("/bin/sh %s" % filename,
                      shell=True, stdout=PIPE).stdout
        else:
            f = os.popen("/bin/sh %s" % filename)
	c1 = f.read()
	f.close()

        (realUpsStuff, ourOsEnvironmentStuff) = re.split('.*%s' % cutHere, c1)
        #print("ourOsEnvironmentStuff = %s" % ourOsEnvironmentStuff)
        exec ourOsEnvironmentStuff

    ############################################################################
    def _getNewPythonEnv(self, cutHere):
        # quoting difficulties for fixing the thing we're about to import:
        #  need to triple-quote the incoming value of the environmental variable
        #  so that multi-line entries are imported correctly
        quote = '"'
        tick = "'"
        tick3 = "%s%s%s" % (tick, tick, tick)
        
        #               fix = re.sub(  "\'",   "\\'",      v)
        the_fix_line = "fix = re.sub( %s\%s%s, %s\\\\%s%s, v)" % (quote,tick,quote, quote,tick,quote)

        #                 print "os.environ['"   +k+"']   = '''" + fix + "'''" 
        the_print_line = "print %sos.environ[%s%s+k+%s%s] = %s%s + fix + %s%s%s" % (quote, tick,quote, quote,tick,
                                                                                   tick3,quote, quote,tick3,quote)
        
        codeLines = []
        codeLines.append('')
        codeLines.append('echo "%s"' % cutHere)
        codeLines.append("cat <<'EOF' | python")
        codeLines.append("import os")
        codeLines.append("import re")
        codeLines.append("envkeys = os.environ.keys()")
        codeLines.append("envkeys.sort()")
        codeLines.append("for k in envkeys:")
        codeLines.append("    v = os.environ.get(k, '')")
        codeLines.append("    %s" % the_fix_line)
        codeLines.append("    %s" % the_print_line)
        codeLines.append("EOF")

        return '\n'.join(codeLines)

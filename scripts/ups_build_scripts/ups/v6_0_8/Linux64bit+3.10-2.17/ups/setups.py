import os
import os.path
import re
import sys


class setups:
    def __init__(self):
        os.environ['UPS_SHELL'] = "sh"
        sdir=os.path.dirname(__file__)
        f = os.popen( '. %s/setups.sh; echo os.environ\\[\\"UPS_DIR\\"\\]=\\"$UPS_DIR\\"; echo os.environ\\[\\"PRODUCTS\\"\\]=\\"$PRODUCTS\\"; echo os.environ\\[\\"SETUP_UPS\\"\\]=\\"$SETUP_UPS\\"' % sdir)
        exec(f.read())
        os.environ['PATH'] = "%s/bin:%s" %(os.environ['UPS_DIR'],os.environ['PATH'])
        f.close()

    def setup(self, arg):
        self.inhaleresults(os.environ["UPS_DIR"] + '/bin/ups setup ' + arg)

    def unsetup(self, arg):
        self.inhaleresults(os.environ["UPS_DIR"] + '/bin/ups unsetup ' + arg)

    def inhaleresults(self, cmd):

        f = os.popen(cmd)
        filename = f.read()
        filename = filename.strip()
        f.close()

        if filename.find('_fail') > 0:
            return

        setup = open(filename,"a")
        setup.write(
            ''' 
echo "--------------cut here-------------"
cat <<'EOF' | python
import os
import re
for v in os.environ.keys():
    if v.find('()') > 0:
        continue
    fix= re.sub( "\'", "\\'", os.environ[v])
    print( "os.environ['"+v+"'] = '" + fix + "'" )
EOF
            ''')
        setup.close()

        f = os.popen("/bin/sh -c '. " + filename + "'")
        c1 = f.read()
        f.close()
        c1 = re.sub( '(?s).*--------------cut here-------------', '', c1)
        #os.environ = {}
        exec(c1)

    def use_python(self,v):

        self.use_package("python", v, "SETUP_PYTHON")

    def use_package(self, p, v, s):
        if ( v == "" ):
            vm = '.*';
        else:
            vm = v;

        if ( not os.environ.has_key(s) or None == re.search(p+' '+vm,os.environ.get(s,''))):
            if os.environ.has_key(s) and environ[s] != '':
                self.unsetup(p)
            self.setup(p + ' ' + v)
            sys.argv.insert(0, os.environ.get('PYTHON_DIR','/usr')+'/bin/python')
            os.execve( os.environ.get('PYTHON_DIR','usr')+'/bin/python', sys.argv, os.environ )

    def ups(self, *args):
        f = os.popen( os.environ['UPS_DIR'] + '/bin/ups ' + ' '.join(args))
        res = f.read()
        f.close()
        return res

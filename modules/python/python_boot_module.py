#
# this is the main boot script for the python module
#


class PythonBoot:
    """The Python Boot Loader"""

    def runScript(self,window,content,filename):
      print "asked to execute content: %s" % content
      script = compile(content,filename,"exec")
      return eval(script)


PyBoot = PythonBoot()

# register our methods directly against python binding
Titanium.set("ti.python.runScript",PyBoot.runScript)



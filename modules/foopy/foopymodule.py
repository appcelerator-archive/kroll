
class Foo:
    """A simple class"""
    i = 12345
    def bar(self):
        print "Foo.bar in python script invoked"
       	return 'hello world'
    def blok(self,fn):
        print "before invoke: " + str(fn)
        fn()
        print "after invoke: " + str(fn)
    def grok(self,obj):
		print "passed to me: " + str(obj.a)
		return obj.f
    def dom(self,dom,i):
        print "before invoke dom"
        el = dom.getElementById(i)
        print "after invoke dom = " + str(el)
        return el
    def win(self,window):
	    return window.title

# test setting to the ti.python.foopy key (implicit)		
Titanium.set("foopy",Foo())

# test setting to the ti.blah.foopy key (explicit)
Titanium.set("ti.blah.foopy",Foo())

print "hello, world from inside Python module!"

f = Foo()
print f.bar()

f.blok(f.bar)

print "python module script now loaded...."

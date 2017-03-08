#coding:utf-8
__author__ = 'kai.xu'

################################################################
# 类的继承和重载
################################################################

class originalClass(object):
    def __init__(self):
        self.word = 'Fuck this bloody world!'
    def printFunction(self):
        print 'Hello World'
    def test(self):
        pass

class myClass(originalClass):
    def __init__(self,parent=None):
        super(originalClass,self).__init__()

class myAnotherClass(myClass):
    def __init__(self,parent=None):
        super(myClass,self).__init__()
    def printFunction(self):
        print 'Fuck this bloody world!'
        super(myAnotherClass, self).printFunction()

if __name__ == '__main__':
    go = myClass()
    run = myAnotherClass()
    run.printFunction()




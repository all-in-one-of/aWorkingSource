class FUN():
    GLOBAL_TEXT = "Hello World"
    def __init__(cls):
        cls.LOCAL_TEXT = "World FUCK UP"
    @classmethod
    def somethingFun(cls):
        print cls.GLOBAL_TEXT
    def somethingGood(cls):
        print cls.LOCAL_TEXT
        print cls.GLOBAL_TEXT
        cls.somethingFun()
if __name__ == '__main__':
    instance = FUN()
    instance.GLOBAL_TEXT = "Hello Kevin"
    instance.somethingGood()
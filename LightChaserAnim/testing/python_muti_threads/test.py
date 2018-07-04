# import Queue
# import threading
# import urllib2

# # called by each thread
# def get_url(q, url):
#     q.put(urllib2.urlopen(url).read())

# def printf(q,text):
#     q.put(urllib2.urlopen(url).read())
#     print "Hello World of %s"%text

# theurls = ["http://baidu.com.com", "https://www.bing.com"]

# q = Queue.Queue()

# for u in theurls:
#     t = threading.Thread(target=printf, args = ("es"))
#     t.daemon = True
#     t.start()

# s = q.get()
# print s

import urllib2 
from multiprocessing.dummy import Pool as ThreadPool 

def printf(text):
    print "Hello World of %s"%text

urls = [
  'http://www.python.org', 
  'http://www.python.org/about/',
  'http://www.onlamp.com/pub/a/python/2003/04/17/metaclasses.html',
  'http://www.python.org/doc/',
  'http://www.python.org/download/',
  'http://www.python.org/getit/',
  'http://www.python.org/community/',
  'https://wiki.python.org/moin/',
]

# make the Pool of workers
pool = ThreadPool(4) 

# open the urls in their own threads
# and return the results
results = pool.map(printf, urls)
print results
# close the pool and wait for the work to finish 
pool.close() 
pool.join() 
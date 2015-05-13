#!/usr/bin/env python
from __future__ import with_statement
from errno import ENOENT
from stat import S_IFDIR, S_IFREG
from sys import argv, exit
from time import time
import os

from fuse import FUSE, FuseOSError, Operations, LoggingMixIn, fuse_get_context

def check(s):  # check if only made up of digit 
  return s[1:].isdigit()

class my_file_sys(LoggingMixIn, Operations):
    def __init__(self):  # start mounting
        print 'start'
    
    def access(self, path, mode):
        print 'access',path
        full_path = '/proc'+path
        if not os.access(full_path, mode):
            raise FuseOSError(errno.EACCES)  
  
    def chmod(self, path, mode):
        full_path = '/proc'+path
        return os.chmod(full_path, mode)

    def chown(self, path, uid, gid):
        full_path = '/proc'+path
        return os.chown(full_path, uid, gid)

    def getattr(self, path, fh=None):
        uid, gid, pid = fuse_get_context()
        if path == '/':
            st = dict(st_mode=(S_IFDIR | 0755), st_nlink=2)
        elif check(path):
            size = 10000
            st = dict(st_mode=(S_IFREG | 0666), st_size=size)
        else:
            raise FuseOSError(ENOENT)
        st['st_ctime'] = st['st_mtime'] = st['st_atime'] = time()
        return st
   
    def read(self, path, size, offset, fh):
        
        encoded = lambda x: ('%s\n' % x).encode('utf-8')
       
        if check(path):
            print 'read....',path
            fh=os.open('/proc'+path+'/status',os.O_RDWR)  # get the status
            offset=0
            length=10000
            os.lseek(fh,offset,os.SEEK_SET)
            tmp_read = os.read(fh,length)        
            return encoded(tmp_read)
        raise RuntimeError('unexpected path: %r' % path)

    def readdir(self, path, fh):  # for ls myproc
        return self.process_list()
    
    def process_list(self):  # get the process id
      pids = []
      for subdir in os.listdir('/proc'):
           if(subdir.isdigit()):
              pids.append(subdir)
      print pids
      return ['.','..']+pids
    
    def open(self, path, flags):
            full_path = '/proc'+path
            print 'open.......',full_path,flags
            return os.open(full_path+'/status', flags)

    def create(self, path, mode, fi=None):
        print 'create...'
        full_path = '/proc'+path
        return os.open(full_path, os.O_WRONLY | os.O_CREAT, mode)

    def write(self, path, data, offset, fh):
        print path,data
        if data[:7]=='goodbye':      # kill by pid
            os.kill(int(path[1:]),9) 
        return 10
    
    def truncate(self, path, length, fh=None):
        print 'truncate',path
        length=1000
        with open('/proc'+path+'/status', 'r+') as f:
            f.truncate(length)
 

    # Disable unused operations:
    access = None    
    getxattr = None
    listxattr = None
    opendir = None   
    releasedir = None
    statfs = None


if __name__ == '__main__':
    if len(argv) != 2:
        print('usage: %s <mountpoint>' % argv[0])
        exit(1)

    fuse = FUSE(my_file_sys(), argv[1], foreground=True)


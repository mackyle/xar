#
# py-xar
# Python Bindings for XAR, the eXtensible ARchiver
#
# Copyright (c) 2005, 2006, 2007 Will Barton <wb@willbarton.com>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions
# are met:
# 
#   1. Redistributions of source code must retain the above copyright 
#      notice, this list of conditions and the following disclaimer.
#   2. Redistributions in binary form must reproduce the above copyright 
#      notice, this list of conditions and the following disclaimer in the 
#      documentation and/or other materials provided with the distribution.
#   3. The name of the author may not be used to endorse or promote products
#      derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
# THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
## TODO: (In no particular order)
##  - Error Handling!! Errors are mostly ignored at the moment
##  - Cache the member list in XarArchive
##  - Implement remaining container methods in XarArchive
##  - General clean-up

## XXX: Importing __builtin__ to use open(), because the local open()
##      function for XarArchives is found first.  Is there a better way?
import os, os.path, __builtin__

__version__ = "0.4"

cdef extern from "xar/xar.h":

    # Phantom structs
    cdef struct __xar_t: 
        pass
    cdef struct __xar_file_t:
        pass
    cdef struct __xar_iter_t:
        pass
    cdef struct __xar_subdoc_t:
        pass

    # Types
    ctypedef __xar_t        *xar_t
    ctypedef __xar_file_t   *xar_file_t
    ctypedef __xar_iter_t   *xar_iter_t
    ctypedef __xar_subdoc_t *xar_subdoc_t
    ctypedef void           *xar_errctx_t
    ctypedef int            (*err_handler)(int severit, int instance, xar_errctx_t ctx, void *usrctx)
    
    # XAR files
    cdef xar_t  xar_open(char *file, int flags)
    cdef int    xar_close(xar_t x)

    # Iterators
    cdef xar_iter_t xar_iter_new()
    cdef void       xar_iter_free(xar_iter_t i)

    # Adding/Extracting files
    cdef xar_file_t xar_add(xar_t x, char *path)
    cdef int        xar_extract(xar_t x, xar_file_t f)
    cdef int        xar_extract_tofile(xar_t x, xar_file_t f, char *path)
    
    # Files in the archive
    cdef xar_file_t xar_file_first(xar_t x, xar_iter_t i)
    cdef xar_file_t xar_file_next(xar_iter_t i)
    cdef char       *xar_get_path(xar_file_t f)
    cdef int        xar_prop_set(xar_file_t f, char *key, char *value)
    cdef int        xar_prop_get(xar_file_t f, char *key, char **value)

    # Subdocs
    cdef xar_subdoc_t   xar_subdoc_new(xar_t x, char *name)
    cdef xar_subdoc_t   xar_subdoc_first(xar_t x)
    cdef xar_subdoc_t   xar_subdoc_next(xar_subdoc_t s)
    cdef char           *xar_subdoc_name(xar_subdoc_t s)
    cdef int            xar_subdoc_copyout(xar_subdoc_t s, char **, int *)
    cdef int            xar_subdoc_copyin(xar_subdoc_t s, char *, int)
    cdef void           xar_subdoc_remove(xar_subdoc_t s)

    # Errors
    cdef void       xar_register_errhandler(xar_t x, err_handler callback, 
                        void *usrctx)
    cdef xar_file_t xar_err_get_file(xar_errctx_t ctx)
    cdef char       *xar_err_get_string(xar_errctx_t ctx)
    cdef int        xar_err_get_errno(xar_errctx_t ctx)
   
# Constants 
READ = 0
WRITE = 1

# Error constants
XAR_SEVERITY_DEBUG    = 1
XAR_SEVERITY_INFO     = 2
XAR_SEVERITY_NORMAL   = 3
XAR_SEVERITY_WARNING  = 4
XAR_SEVERITY_NONFATAL = 5
XAR_SEVERITY_FATAL    = 6
XAR_ERR_ARCHIVE_CREATION   = 1
XAR_ERR_ARCHIVE_EXTRACTION = 2

class XarError(Exception):  pass
class XarCreationError(XarError): pass
class XarExtractionError(XarError): pass
class XarFileError(XarError): pass
class XarArchiveError(XarError): pass
class XarDocError(XarError): pass

# Sort of a clone of xar's err_callback function
cdef int _error_callback(int sev, int err, xar_errctx_t ctx, void *usrctx):
    cdef xar_file_t f
    cdef char *str
    cdef int e

    str = xar_err_get_string(ctx)
    e = xar_err_get_errno(ctx)
    f = xar_err_get_file(ctx)

    if f: 
        file = xar_get_path(f)
    else: 
        file = ""

	# XXX: for some reason, python claims an error raised in this
    # function is ignored? Why?
    
    # Create the exception, then we cycle through severities to decide
    # what to do with it.
    exception = None
    if err == XAR_ERR_ARCHIVE_CREATION:
        exception = XarCreationError("%s:%s" % (str, file))
    elif err == XAR_ERR_ARCHIVE_EXTRACTION:
        exception = XarExtractionError("%s:%s" % (str, file))
    else:
        exception = XarError(s)

    # XXX: What should we do with non-fatal errors?
#    if sev == XAR_SEVERITY_DEBUG or sev == XAR_SEVERITY_INFO: pass
#    elif sev == XAR_SEVERITY_WARNING: 
#        raise exception
#    elif sev == XAR_SEVERITY_NORMAL: 
#        raise exception
#    elif sev == XAR_SEVERITY_NONFATAL: 
#        raise exception
#    elif sev == XAR_SEVERITY_FATAL:
#        raise exception
    
    raise exception

# Create a XarInfo object based on the properties of a xar_file_t
cdef _xarfile_get_info(xar_file_t c_file):
    cdef char *name, *type, *mode, *uid, *user, *gid, *group, \
            *atime, *mtime, *ctime
    name = xar_get_path(c_file)
    xar_prop_get(c_file, "type", &type)
    xar_prop_get(c_file, "mode", &mode)
    xar_prop_get(c_file, "uid", &uid)
    xar_prop_get(c_file, "user", &user)
    xar_prop_get(c_file, "gid", &gid)
    xar_prop_get(c_file, "group", &group)
    xar_prop_get(c_file, "atime", &atime)
    xar_prop_get(c_file, "mtime", &mtime)
    xar_prop_get(c_file, "ctime", &ctime)
    
    return XarInfo(name, type, uid, user, gid, group, atime, mtime, ctime)

cdef _xarfile_get_subdoc(xar_subdoc_t c_subdoc):
    cdef char *name, *xmlstring
    cdef int size

    name = xar_subdoc_name(c_subdoc)
    #xar_subdoc_copyout(xar_subdoc_t s, unsigned char **, unsigned int*)
    xar_subdoc_copyout(c_subdoc, &xmlstring, &size)

    return XarSubdoc(name, xmlstring)

# Open a Xar archive
# Creates an instance of XarArchive, and returns it
def open(name, mode = 'r'):
    return XarArchive(name, mode)

# Close an instance of XarArchive
def close(archive):
    archive.close()
   
cdef class XarArchive:
    """ This class provides an interface to a xar archive file.  Each
        archive member is represented by a XarInfo object."""
    cdef xar_t _c_xar_t
    cdef int _mode 
    cdef char *name

    def __init__(self, name, mode = 'r'):
        """ Open a xar archive with filename 'name'.  Mode is either 'r'
            or 'w' for read and write, respectively. """
        self._c_xar_t = NULL
        self.name = name
        self.mode = mode
        self.open()

    def __dealloc__(self):
        if self._c_xar_t != NULL: self.close()

    def __str__(self):
        return "XarArchive: %s (mode %s)" % (self.name, self.mode)

    property mode:
        """ mode property to set the private _mode integer to the
            friendly string value """
        def __get__(self):
            if self._mode == WRITE:
                return 'w'
            return 'r'
        def __set__(self, value):
            if value == 'w':
                self._mode = WRITE
            else:
                self._mode = READ

    def open(self):
        """ Open the XarArchive """
        if self._c_xar_t != NULL:
            return

        cdef xar_t c_xar_t
        c_xar_t = xar_open(self.name, self._mode)
        if c_xar_t == NULL:
            raise XarArchiveError, "Unable to open xar archive"
        xar_register_errhandler(c_xar_t, _error_callback, NULL)
        self._c_xar_t = c_xar_t

    def close(self): 
        """ Close the XarArchive.  This method is called implicitly upon
            deallocation, if not explicitly prior. """
        xar_close(self._c_xar_t)
        self._c_xar_t = NULL

    ####################################################################
    # Member Access
        
    # The following method, getmembers, is the core of the functionality
    # of this class.  All other query methods are based on getmembers,
    # including the container methods.
    def getmembers(self):
        """ Return the members as a list of XarInfo objects. """
        ## XXX: Cache the member list?  Since much of the functionality
        ## of the rest of the module depends on this, we shouldn't have
        ## to continually read from the xarfile.
        cdef xar_iter_t c_iter
        cdef xar_file_t c_file

        result = []

        c_iter = xar_iter_new()
        c_file = xar_file_first(self._c_xar_t, c_iter)
        while c_file != NULL:
            info = _xarfile_get_info(c_file)
            result.append(info)
            c_file = xar_file_next(c_iter)
        xar_iter_free(c_iter)

        return result

    def getnames(self):
        """ Return the members as a list of their names. """
        names = []
        for m in self.getmembers(): 
            names.append(m.name)
        return names

    # Python Container methods for the XarArchive
    # These cause the archive to be accessible as a Python container, in
    # this case, dictionary-like, using member names as keys.  Other
    # dictionary-like methods are defined for further compatibilty.
    # 
    # These are all ultimately wrappers of some kind around getmembers().
    def __len__(self): 
        """ Return the number of members in the archive """
        members = self.getmembers()
        return len(members)

    def __getitem__(self, key): 
        """ Return the member matching the given name """
        return dict(self.items()).__getitem__(key)

    def __iter__(self): 
        """ Return an iterator over the archive members """
        return self.getmembers().__iter__()

    def __contains__(self, item): 
        """ Return whether the archive contains the given XarInfo object """
        return (item in self.getmembers())

    def items(self):
        """ Return a list of tuples of (name, member) for the members in
            the archive. """
        items = []
        for m in self.getmembers():
            items.append((m.name, m))
        return items
        
    def has_key(self, key):
        """ Return whether the archive contains the given name """
        return key in self.getnames()

    def get(self, key, default = None):
        """ Return the member for the given name, or the given default
            if the name doesn't exist """
        return dict(self.items()).get(key, default)

    def iteritems(self):
        return dict(self.items()).itemitems()

    def iterkeys(self):
        return dict(self.items()).itemkeys()

    def itervalues(self):
        return dict(self.items()).itemvalues()

    # Container methods for which we already have identical methods
    def keys(self): return self.getnames()
    def values(self): return self.getmembers()

    ## XXX: Writable container methods need implementing
    def __setitem__(self, key, value): pass
    def __delitem__(self, key): pass
    def setdefault(self, key, default): pass
    def pop(self, key, default): pass
    def popitem(self): pass
    def update(self, e, **f): pass

    def getmember(self, name): 
        """ Return a XarInfo object for the member """
        ## Return the first XarInfo instance who's name matches the
        ## given name
        for member in self.getmembers():
            if member.name == name: 
                return member
        # If we arrived here, we did not find the member
        raise KeyError, "Member not found"

    ####################################################################
    # Subdoc access

    def getsubdoc(self, name):
        """ Return a XarSubdoc object for the subdoc of the given name """
        for subdoc in self.getsubdocs():
            if subdoc.name == name:
                return subdoc
        # If we arrived here, we did not find the subdoc
        raise KeyError, "Subdoc not found"

    def getsubdocs(self):
        """ Return a list of XarSubdoc objects for each subdoc in the
            archive."""
        cdef xar_subdoc_t c_subdoc

        result = []

        c_subdoc = xar_subdoc_first(self._c_xar_t)
        while c_subdoc != NULL:
            subdoc = _xarfile_get_subdoc(c_subdoc)
            result.append(subdoc)
            c_subdoc = xar_subdoc_next(c_subdoc)

        return result

    def getsubdocnames(self):
        """ Return a list of the names of each subdoc in the archive. """
        cdef xar_subdoc_t c_subdoc

        result = []

        c_subdoc = xar_subdoc_first(self._c_xar_t)
        while c_subdoc != NULL:
            result.append(xar_subdoc_name(c_subdoc))
            c_subdoc = xar_subdoc_next(c_subdoc)

        return result

    ####################################################################
    # Extraction

    def extract(self, member = None, path = None):
        """ Extract the member to the given path, or the current
            directory if no path is given. If no member is given, every 
            file in the archive is extracted to the given path, or the 
            current directory. """
        ## XXX: Because we need the xar_file_t object for extraction, we 
        ## have to duplicate the code from getmembers().  Should find a 
        ## better way to do this.
        cdef xar_iter_t c_iter
        cdef xar_file_t c_file

        c_iter = xar_iter_new()
        c_file = xar_file_first(self._c_xar_t, c_iter)
        while c_file != NULL:
            info = _xarfile_get_info(c_file)

            if not member or (member == info.name):
                if path:
                    if (xar_extract_tofile(self._c_xar_t, c_file, path) != 0):
                        raise XarExtractionError,  \
                            "Error extracting " + info.name
                else:
                    if (xar_extract(self._c_xar_t, c_file) != 0):
                        raise XarExtractionError,  \
                            "Error extracting " + info.name

            c_file = xar_file_next(c_iter)

        xar_iter_free(c_iter)

    ####################################################################
    # Mutation

    def add(self, name, recursive = True): 
        """ Add a file (or directory) to the archive, if the archive is
            writable.  If the name is a directory, and recursive is
            True, then add the directory recursively. """
        cdef xar_file_t c_file

        if self.mode != 'w': 
            raise XarArchiveError, "archive is not writable"
       
        # Normalize the path, if necessary
        name = os.path.expanduser(name)
        name = os.path.expandvars(name)
        name = os.path.normpath(name)

        # Add the file
        c_file = xar_add(self._c_xar_t, name)
        if c_file == NULL:
            raise XarFileError, "file does not exist: %s" % name
        info = _xarfile_get_info(c_file)

        # If the file is a directory, and recursive is true, recurse
        if info.isdir() and recursive:
            for file in os.listdir(name):
                self.add(os.path.join(name, file), recursive = True)

        return info

    def addfile(self, xarinfo): 
        raise NotImplementedError

    def getxarinfo(self, name): 
        """ Create a XarInfo object for the given file name that is NOT
            in the XarArchive. """
        raise NotImplementedError

    def addsubdoc(self, subdoc):
        """ Adds the given subdoc to the xar archive.  Subdoc is assumed
            to be an instance of XarSubdoc, but if not, it is assumed to
            be a path to an xml file that can be used to create a
            XarSubdoc instance. """
        cdef xar_subdoc_t c_subdoc

        # Note: This does catch subclasses of XarDoc as well
        if not isinstance(subdoc, XarDoc):
            subdoc = XarSubdoc.fromfile(subdoc)

        if self.mode != 'w':
            raise XarArchiveError, "archive is not writable"

        name = subdoc.name
        xmlstring = subdoc.toxml()
        size = len(xmlstring)

        # Add the subdoc
        c_subdoc = xar_subdoc_new(self._c_xar_t, name)
        if c_subdoc == NULL:
            raise XarError, "unable to add subdoc to the archive"
        xar_subdoc_copyin(c_subdoc, xmlstring, size)

        return subdoc

    def removesubdoc(self, subdoc):
        """ Remove the subdoc of the given name from the archive. """
        cdef xar_subdoc_t c_subdoc

        # Note: This does catch subclasses of XarDoc as well
        if isinstance(subdoc, XarDoc): name = subdoc.name
        else: name = subdoc

        c_subdoc = xar_subdoc_first(self._c_xar_t)
        while c_subdoc != NULL:
            if name == xar_subdoc_name(c_subdoc):
                xar_subdoc_remove(c_subdoc)
                return
            c_subdoc = xar_subdoc_next(c_subdoc)

        raise KeyError, "Subdoc not found in archive"


class XarInfo(object):

    def __init__(self, name, type = '', mode = '', uid = '', user = '',
            gid = '', group = '', atime = '', mtime = '', ctime = ''): 
        self.name = name
        self.type = type
        self.mode = mode
        self.uid = uid
        self.user = user
        self.gid = gid
        self.group = group
        self.atime = atime
        self.mtime = mtime
        self.ctime = ctime

    def __str__(self):
        return self.name
    def __repr__(self):
        return self.__str__()

    def frombuf(self): 
        raise NotImplementedError
    def tobuf(self): 
        raise NotImplementedError

    def isfile(self): 
        if self.type == 'file': 
            return True
        return False

    def isreg(self): 
        return self.isfile()

    def isdir(self): 
        if self.type == 'directory': 
            return True
        return False

    def issym(self): 
        raise NotImplementedError
    def islink(self): 
        raise NotImplementedError
    def ischr(self): 
        raise NotImplementedError
    def isblk(self): 
        raise NotImplementedError
    def isfifo(self): 
        raise NotImplementedError
    def isdev(self): 
        raise NotImplementedError

def XarDoc_fromfile(cls, filename):
    """ Load an XML file as a XarDoc (for use in adding subdocs to
        archives.) """
    ## XXX: Importing __builtin__ to use open(), because the local
    ##      open() function for XarArchives is found first.  Is there a
    ##      better way?
    fd = __builtin__.open(filename)
    xmlstring = fd.read()
    fd.close()
    name = os.path.splitext(filename)[0]
    return cls(name, xmlstring)

class XarDoc(object):
    """ Class representing all XML documents related to a xar archive,
        including subdocs and the table of contents. """

    fromfile = classmethod(XarDoc_fromfile)

    def __init__(self, name, xml):
        self.name = name
        # XXX: c14n for comparison
        self._xml = xml

    def __cmp__(self, other):
        return self.name == other.name and self._xml == other._xml

    def toxml(self):
        """ Return the xml string for the document """
        return self._xml
   
    def tofile(self, filename):
        """ Store the xml string to a file by the given filename """
        ## XXX: Importing __builtin__ to use open(), because the local
        ##      open() function for XarArchives is found first.  Is 
        ##      there a better way?
        fd = __builtin__.open(filename)
        fd.write(self._xml)
        fd.close()
   
    def parse(self, parser_function, args = None):
        """ Parse the document's xml string using the given function.
            This assumes the function takes a single string argument """
        return parser_function(self._xml)

class XarSubdoc(XarDoc):
    pass

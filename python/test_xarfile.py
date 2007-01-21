# -*- coding: utf-8 -*-
#
# Copyright (c) 2006, 2007 Will Barton.  All rights reserved.
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

import unittest
import tempfile
import os, os.path
import random

import xarfile
from xarfile import XarError, XarCreationError, XarExtractionError
from xarfile import XarFileError, XarArchiveError

subdoc = '''<?xml version='1.0' encoding="UTF-8"?>
<myns:document xmlns:myns="http://foo.bar/myns" 
               xmlns:somens="http://some.site/somens">
    <myns:someelm>
        Some Text Here
    </myns:someelm>
    <somens:parent>
        <somens:child>
            <myns:stuff>Woot!</myns:stuff>
        </somens:child>
    </somens:parent>
</myns:document>'''

def makeTestFile(dir = ""):
    """ Create a test file, filled with random data. """
    fd, name = tempfile.mkstemp(dir = dir)
    testfile = os.fdopen(fd, "w+b")
    lines = ["Test of xarfile line %d.\n" % i for i in xrange(0, 1000)]
    testfile.writelines(lines)
    testfile.close()
    return name 

class XarArchiveTestCase(unittest.TestCase):

    def setUp(self):
        # The filename for our temporary xar file
        self.archive = os.path.join(tempfile.gettempdir(), "%sxar%sxar" %
                (tempfile.gettempprefix(), os.extsep))

        # A test subdoc object
        self.test_subdoc = xarfile.XarSubdoc("testdoc", subdoc)
         
        # A single file to xar up
        self.test_file = makeTestFile(dir = os.path.join(os.sep,
            tempfile.gettempprefix()))
            
        # A non-existent filename
        self.test_dne = os.path.join(tempfile.gettempprefix(), "dne" + \
                str(random.random()))

        # Create a few temporary files in a directory to xar up recursively
        # XXX: Robustify this to include multiple levels of directories
        self.test_dir = tempfile.mkdtemp()
        self.test_files = []
        for i in xrange(2):
            self.test_files.append(makeTestFile(dir = self.test_dir))
        self.test_members = self.test_files + \
                [tempfile.gettempprefix(), self.test_dir, self.test_file]
        self.test_members = map(lambda m: m.lstrip(os.sep), self.test_members)

    def tearDown(self):
        # Remove the test xar file, if it exists
        if os.path.exists(self.archive):
            os.remove(self.archive)

        # Remove the test file
        os.remove(self.test_file)
 
        # Remove the test directory
        for f in self.test_files: os.remove(f)
        os.rmdir(self.test_dir)

class XarArchiveWriteTestCase(XarArchiveTestCase):

    def setUp(self):
        XarArchiveTestCase.setUp(self)
        self.xarf = xarfile.XarArchive(self.archive, 'w')

    def tearDown(self):
        self.xarf.close()
        XarArchiveTestCase.tearDown(self)

    def testAdd(self):
        self.xarf.add(self.test_file, recursive = False)
        self.failUnlessRaises(XarError, self.xarf.add, self.test_dne)
        self.xarf.add(self.test_dir, recursive = True)
        # XXX: test non-recursive addition of a directory

    ###################################################################
    # Subdocs

    def testAddSubdoc(self):
        # Add to a new self.archive
        self.xarf.addsubdoc(self.test_subdoc) # Should succeed
        s = self.xarf.getsubdoc(self.test_subdoc.name)
        # XXX: This will fail until the xml is normalized within XarDoc
        self.failUnlessEqual(s, self.test_subdoc)

    def testRemoveSubdoc(self):
        # XXX: This fails for some reason, when it shouldn't.  I think
        # it has something to do with write mode.
        self.xarf.addsubdoc(self.test_subdoc) 
        self.xarf.removesubdoc("testdoc")
        self.failUnlessRaises(KeyError, self.xarf.removesubdoc, "dnedoc")


class XarArchiveReadTestCase(XarArchiveTestCase):

    def setUp(self):
        XarArchiveTestCase.setUp(self)

        # Create the test xar self.archive in a predictable manner. This is
        # intended to by used by query-related methods. 
        self.xarf = xarfile.XarArchive(self.archive, 'w')
        self.xarf.add(self.test_dir, recursive = True)
        self.xarf.add(self.test_file)
        self.xarf.addsubdoc(self.test_subdoc)
        self.xarf.close()

        self.xarf = xarfile.XarArchive(self.archive, 'r')
        
    def tearDown(self):
        self.xarf.close()

        XarArchiveTestCase.tearDown(self)

    ###################################################################
    # Member Access
    
    def testGetMembers(self):
        members = self.xarf.getmembers()
        self.failUnless(len(members) == len(self.test_members))
        #self.failUnless(self.test_member in members)

    def testGetNames(self):
        names =  self.xarf.getnames()
        for f in self.test_members:
            self.failUnless(f in names)
        self.failUnless(self.test_file.lstrip(os.sep) in names)

    def testGetMember(self):
        self.failUnlessRaises(KeyError, self.xarf.getmember, self.test_dne)
        self.xarf.getmember(self.test_file.lstrip(os.sep))
    
    def testAdd(self):
        self.failUnlessRaises(XarArchiveError, self.xarf.add, self.test_file)

    ###################################################################
    # Extraction

    def testExtract(self):
        extract_dir = tempfile.mkdtemp()
        extract_filename = os.path.join(extract_dir, 
                os.path.basename(self.test_file))

        self.xarf.extract(member = self.test_file.lstrip(os.sep), 
                path = extract_filename)
        self.failUnless(os.path.exists(extract_filename))

        self.test_file_lines = open(self.test_file).readlines()
        extract_file_lines = open(extract_filename).readlines()
        self.failUnless(self.test_file_lines == extract_file_lines)

        # XXX: Also check the checksums of the file, the various
        # metadata, etc.  This should be provided by XarInfo.__cmp__,
        # creating a new XarInfo for each file.

        # XXX: How do we generate one of these?
        # self.failUnlessRaises(XarExtractionError, self.xarf.extract)

        os.remove(extract_filename)
        os.rmdir(extract_dir)
    
    ###################################################################
    # Subdocs

    def testGetSubdoc(self):
        self.xarf.getsubdoc("testdoc") # Should succeed
        self.failUnlessRaises(KeyError, self.xarf.getsubdoc, "dnedoc")

    def testGetSubdocs(self):
        subdocs = self.xarf.getsubdocs() # Should succeed
        #self.failUnless(self.test_subdoc in subdocs)
        #self.failUnless(XarSubdoc("dnedoc", subdoc) not in subdocs)

    def testGetSubdocNames(self):
        names = self.xarf.getsubdocnames() # Should succeed
        self.failUnless("testdoc" in names)
        self.failUnless("dnedoc" not in names)

if __name__ == "__main__":
    suite = unittest.TestSuite()

    suite.addTest(XarArchiveWriteTestCase("testAdd"))
    suite.addTest(XarArchiveWriteTestCase("testAddSubdoc"))
    suite.addTest(XarArchiveWriteTestCase("testRemoveSubdoc"))
    suite.addTest(XarArchiveReadTestCase("testAdd"))
    suite.addTest(XarArchiveReadTestCase("testGetMembers"))
    suite.addTest(XarArchiveReadTestCase("testGetNames"))
    suite.addTest(XarArchiveReadTestCase("testGetMember"))
    suite.addTest(XarArchiveReadTestCase("testExtract"))
    suite.addTest(XarArchiveReadTestCase("testGetSubdoc"))
    suite.addTest(XarArchiveReadTestCase("testGetSubdocs"))
    suite.addTest(XarArchiveReadTestCase("testGetSubdocNames"))

    unittest.TextTestRunner(verbosity=2).run(suite)

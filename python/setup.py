from distutils.core import setup
from distutils.extension import Extension
from Pyrex.Distutils import build_ext

xarfile = Extension('xarfile',
        libraries = ['xar'],
        sources = ['xarfile.pyx'])

setup(
        name = 'pyxar', 
        version = '0.3.0',
        description = 'Python bindings for XAR, the eXtensible ARchiver', 
        author = 'Will Barton',
        author_email = 'wbb4@opendarwin.org',
        url = 'http://www.opendarwin.org/projects/xar/python',
        license = 'BSD-style License',
        ext_modules = [xarfile],
        cmdclass = {'build_ext':build_ext}
)

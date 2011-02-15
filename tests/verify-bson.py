#! /usr/bin/python
from datetime import datetime
import sys, re

skip = False
try:
    from bson import BSON
    from bson.binary import Binary
except:
    print "1..0 # SKIP PyMongo version is too old"
    exit (1)

def verify_bson (name, source, ok, no):
    print "# %s" % name
    if skip:
        print "ok %d - # SKIP " % no
        return

    d = BSON.encode (ok)
    s = BSON (source.rstrip ().decode ('string_escape'))
    s = BSON.encode (s.decode ())
    if s.decode () == d.decode ():
        print "ok %d" % no
    else:
        print "# source: %s; dest: %s" % (s.decode (), d.decode ())
        print "not ok %d" % no

def verify_regexp (name, source, ok, no):
    print "# %s" % name
    if skip:
        print "ok %d # skipped" % no
        return

    s = BSON (source.rstrip ().decode ('string_escape'))
    s = BSON.encode (s.decode ())
    sr = s.decode ()['regexp']
    if sr.pattern == ok.pattern and sr.flags == ok.flags:
        print "ok %s" % no
    else:
        print "# source: %s; dest: %s" % (sr.pattern, ok.pattern)
        print "not ok %s" % no
        
def bson_build_base (f):
    verify_bson ("bson_empty", f.readline (), {}, 1)
    verify_bson ("bson_string", f.readline (),
                 {"hello": "world"}, 2)
    verify_bson ("bson_string_len", f.readline (),
                 {"goodbye": "cruel world"}, 3)
    verify_bson ("bson_double", f.readline (),
                 {"double": 3.14}, 4)
    verify_bson ("bson_boolean", f.readline (),
                 {"TRUE": True, "FALSE": False}, 5)
    verify_bson ("bson_utc_datetime", f.readline (),
                 {"date": datetime.utcfromtimestamp (1294860709)}, 6)
    verify_bson ("bson_null", f.readline (),
                 {"null": None}, 7)
    verify_bson ("bson_int32", f.readline (),
                 {"int32": 1984}, 8)
    verify_bson ("bson_int64", f.readline (),
                 {"int64": 9876543210}, 9)
    verify_regexp ("bson_regexp", f.readline (),
                   re.compile ("foo.*bar", re.I), 10)
    verify_bson ("bson_binary_0", f.readline (),
                 {"binary0": Binary ("foo\x00bar", 0)}, 11)
    verify_bson ("bson_binary_2", f.readline (),
                 {"binary2": Binary ("foo\x00bar", 2)}, 12)
    if f.readline () != '':
        print "# garbage after tests"
        print "not ok %d" % 13
        exit (1)

def bson_build_compound (f):
    # Everything from bson_build_simple in one, except the empty bson.
    verify_bson ("bson_compound_flat", f.readline (),
                 {"hello": "world",
                  "goodbye": "cruel world",
                  "double": 3.14,
                  "TRUE": True,
                  "FALSE": False,
                  "date": datetime.utcfromtimestamp (1294860709),
                  "null": None,
                  "int32": 1984,
                  "int64": 9876543210
                 }, 13)

    # A complex document, with arrays and subdocuments
    verify_bson ("bson_compound_nested", f.readline (),
                 { "user":
                       { "name": "V.A. Lucky", "id": 12345 },
                   "posts": [ {"title": "Post #1",
                               "date": datetime.utcfromtimestamp (1294860709),
                               "comments": ["first!", "2nd!", "last!"],
                               },
                              {"title": "Post #2",
                               "date": datetime.utcfromtimestamp (1294860709),
                               }
                            ]
                 }, 14)

def plan (f):
    print "1..14"
    pass

eval (sys.argv[1] + '(sys.stdin)')

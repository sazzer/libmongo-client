#! /usr/bin/python
from datetime import datetime
import sys

skip = False
try:
    from bson import BSON
except:
    skip = True

def verify_bson (name, source, ok):
    if skip:
        print "SKIP: %s" % name
        return

    d = BSON.encode (ok)
    s = BSON (source.rstrip ().decode ('string_escape'))
    s = BSON.encode (s.decode ())
    if s.decode () == d.decode ():
        print "PASS: %s" % name
    else:
        print "FAIL: %s" % name
        print "# source: %s" % s.decode ()
        print "# dest  : %s" % d.decode ()
        exit (1)

def bson_build_simple (f):
    verify_bson ("bson_empty", f.readline (), {})
    verify_bson ("bson_string", f.readline (),
                 {"hello": "world"})
    verify_bson ("bson_string_len", f.readline (),
                 {"goodbye": "cruel world"})
    verify_bson ("bson_double", f.readline (),
                 {"double": 3.14})
    verify_bson ("bson_boolean", f.readline (),
                 {"TRUE": True, "FALSE": False})
    verify_bson ("bson_utc_datetime", f.readline (),
                 {"date": datetime.utcfromtimestamp (1294860709)})
    verify_bson ("bson_null", f.readline (),
                 {"null": None})
    verify_bson ("bson_int32", f.readline (),
                 {"int32": 1984})
    verify_bson ("bson_int64", f.readline (),
                 {"int64": 9876543210})
    if f.readline () != '':
        print "FAIL: garbage after tests"
        exit (1)

def bson_build_complex (f):
    # Everything from bson_build_simple in one, except the empty bson.
    verify_bson ("bson_complex_1", f.readline (),
                 {"hello": "world",
                  "goodbye": "cruel world",
                  "double": 3.14,
                  "TRUE": True,
                  "FALSE": False,
                  "date": datetime.utcfromtimestamp (1294860709),
                  "null": None,
                  "int32": 1984,
                  "int64": 9876543210
                 })

    # A complex document, with arrays and subdocuments
    verify_bson ("bson_complex_2", f.readline (),
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
                 })

eval (sys.argv[1] + '(sys.stdin)')

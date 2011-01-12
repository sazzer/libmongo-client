#! /usr/bin/python
from bson import BSON

def bson_build_simple ():
    # bson_empty
    b = BSON.encode ({})
    print (b.__repr__ ())

    # bson_string
    b = BSON.encode ({"hello": "world"})
    print (b.__repr__ ())

    # bson_string_len
    b = BSON.encode ({"goodbye": "cruel world"})
    print (b.__repr__ ())

    # bson_double
    b = BSON.encode ({"double": 3.14})
    print (b.__repr__ ())

    # bson_boolean
    b = BSON.encode ({"TRUE": True, "FALSE": False})
    print (b.__repr__ ())

bson_build_simple ()

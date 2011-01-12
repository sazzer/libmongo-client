#! /usr/bin/python
from bson import BSON
from datetime import datetime

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

    # bson_utc_datetime
    b = BSON.encode ({"date": datetime.utcfromtimestamp (1294860709)})
    print (b.__repr__ ())

    # bson_null
    b = BSON.encode ({"null": None})
    print (b.__repr__ ())

    # bson_int32
    b = BSON.encode ({"int32": 1984})
    print (b.__repr__ ())

    # bson_int64
    b = BSON.encode ({"int64": 9876543210})
    print (b.__repr__ ())

bson_build_simple ()

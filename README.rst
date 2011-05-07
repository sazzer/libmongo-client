libmongo-client
===============

This is an alternative C driver for `MongoDB`_, with slightly
different goals than the official one.

We're not so strict on dependencies (we're using `glib`_), and the API
is very different too.

Features
--------

The main purpose of this library is to provide a well maintained
implementation, that suits a wider range of needs than the original:

* Ability to easily construct mongodb commands, to be sent at a later
  time (comes in handy when one is trying to write from a separate
  thread).
* C-like error handling. No toying around with exceptions.
* Comprehensive test suite, with over 90% code coverage, and
  increasing.
* Strict error handling.
* ReplicaSet support, with support for automatic reconnecting and
  discovery.
* Connection pooling.
* Well documented API.

Requirements
------------

Apart from `glib`_, there are no other hard dependencies. Though, one
will need `OpenSSL`_ for authentication support, and `Perl`_ (with a
suitable version of Test::Harness, along with the prove utility) to
run the test suite.

To build the documentation, `Doxygen`_ will be needed too.

Installation
------------

The library follows the usual autotools way of installation:

::

 $ git clone git://github.com/algernon/libmongo-client.git
 $ cd libmongo-client
 $ ./autogen.sh
 $ ./configure && make && make install

License
-------

Although the code is not based on any other driver, it is released
under the same Apache License, version 2.0 (included as the file
LICENSE).

.. _MongoDB: http://www.mongodb.org/
.. _glib: http://developer.gnome.org/glib/
.. _OpenSSL: http://www.openssl.org/
.. _Perl: http://www.perl.org/
.. _Doxygen: http://www.stack.nl/~dimitri/doxygen/

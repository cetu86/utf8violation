utf8violation
=============

motivation
----------

In Unix filenames can be anything. 
As long as a filename doesn't contain the "/"-character or a 0-byte, any binary-garbage forms a legal filename.
This design choice is inherently flawed. It puts the user in charge of making sure filenames are sane.

This program helps the user detect filenames that contain non-utf8 characters or other not alllowed characters.
Currenty not allowed are characters from the
unicode [C0 and C1 sets](https://en.wikipedia.org/wiki/C0_and_C1_control_codes), except for space, #(32).
You can easily modify this behaviour by changing the "is_not_printable"-function.


usage
-----

This program ist called with one argument: a directory as a root from which on it starts recursively scanning
the directory tree for illegal filenames.


output
------

This program outputs every illegal filename, one per line.
Illegal characters are escaped. Their byte-value ist printed as #(n), where n is a number from 1 to 255.  
So the "\n"-Character would be #(10). A latin-1 umlaut 'ä' becomes #(228).  
For proper escaping any character '#' is printed as #(35).

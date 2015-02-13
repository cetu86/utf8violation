utf8violation
=============

motivation
----------

In Unix filenames can be anything. 
As long as a filename doesn't contain the "/"-character or a 0-byte, any binary-garbage forms a legal filename.
This design choice is inherently flawed. It puts the user in charge of making sure filenames are sane.

This program helps the user detect and repair filenames that contain non-utf8 characters or other not alllowed characters.
Currenty not allowed are characters from the
unicode [C0 and C1 sets](https://en.wikipedia.org/wiki/C0_and_C1_control_codes), except for space, #(32).
You can easily modify this behaviour by changing the "is_not_printable"-function.


usage
-----

Usage: ./utf8violation [MODE] [DIRECTORY]

mode can be either 
*  -r  report mode (default): print all violating filenames
*  -a  auto mode: repair filesystem by escaping all violating filenames
*  -i  interactive mode: let the user enter replacement filenames

directory specifies the root for a recursive directory tree walk. 
directory defaults to the current directory.


output
------

In Report mode this program outputs every illegal filename, one per line.
Illegal characters are escaped. Their byte-value ist printed as #(n), where n is a number from 1 to 255.  
So the "\n"-Character would be #(10). A latin-1 umlaut 'ä' becomes #(228).  
For proper escaping any character '#' is printed as #(35).

In auto mode the only output is to stderr in case renaming a file failed.

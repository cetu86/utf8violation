utf8violation
=============

motivation
----------

In Unix filenames can be anything. 
As long as a filename doesn't contain the "/"-character or a 0-byte, any binary-garbage forms a legal filename.
This design choice is inherently flawed. It puts the user in charge of making sure filenames are sane.

This program helps the user detect and repair filenames that contain non-utf8 characters or other not alllowed characters.
Currenty not allowed are characters from the
unicode [C0 and C1 sets](https://en.wikipedia.org/wiki/C0_and_C1_control_codes), except for space, `#(32)`.
You can easily modify this behaviour by changing the "is_not_printable"-function.


usage
-----

Usage:

     ./utf8violation [MODE] [DIRECTORY] [COMMAND and it's ARGUMENTS...]

mode can be either     

| flag | mode              | behaviour                                              |
|---|----------------------|--------------------------------------------------------|
|-r | report mode (default)| print all violating filenames                          |
|-a | auto mode            | repair filesystem by escaping all violating filenames  |
|-i | interactive mode     | let the user enter replacement filenames               |
|-e | external mode        | use command's output for replacement filename.         |

`DIRECTORY` specifies the root for a recursive directory tree walk. 
It defaults to the current directory.

### external mode

The command used in external mode has to consume one line on stdin, which
will be the escaped violating filename and respond on stdout with
the replacement filename, followed by newline `"\n"`. Any exit code other than `0`
will result in the termination of utf8violation.
The external command may signal utf8violation to skip the current file by
returning an empty line on stdout.
If the external command returns invalid utf-8, the current file will be skipped as well.

There is an example in `share/foreign2utf8.pl`, 
that uses [CP1252.TXT](http://unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP1252.TXT)
to convert CP1252 encoded filenames into utf-8. 
Use it like this

    ./utf8violation -e $DIRECTORY perl share/foreign2utf8.pl share/CP1252.TXT


output
------

In report mode this program outputs every illegal filename, one per line.
Illegal characters are escaped. Their byte-value ist printed as `#(n)`, where `n` is a number from 1 to 255.  
So the `"\n"`-Character would be `#(10)`. A latin-1 umlaut `'ä'` becomes `#(228)`.  
For proper escaping any character `'#'` is printed as `#(35)`.

In auto mode the only output is to stderr in case renaming a file failed.


requirements
------------

This program requires [The GNU C Library](http://www.gnu.org/software/libc/)
for its quite versatile [nftw](http://www.gnu.org/software/libc/manual/html_node/Working-with-Directory-Trees.html#Working-with-Directory-Trees) function.

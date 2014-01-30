PSTR
====

PSTR is an abbreviation for PRESENTER(sounds awkward, huh?). PRESENTER is a utility program with the aim of helping developers present their codes to the general audience. It was inspired by the (not-so) popular Linux utility program pv, especially when combined with the '-qL 10' switch, it outputs the text/code file piped to it to the standard output. This can be useful in some other ways, like when showcasing your code to a group of students, friends or colleagues. The program with the right switches, starts displaying the code in the text file and may automatically pause at a specified time given to the CLP or continue displaying the text file until SPACEBAR is hit. It can also be used like a screensaver, it shows a borderless/fullscreen window and immediately starts to "type" out all source code files specified during installation.

What's been done so far?
========================
As at Thursday(30th January 2014).
I've finished with the command-line parser(CLP) and with getting all text files out of a given folder(recursively down the directory). The CLP source code is attributed to Théophile BASTIAN (a.k.a. Tobast), on his wonderful work on the CLI of pastebin source code. The FileHandler code handles the names of all the files in a given directory.

Platform
========
The utility program is(was) written on Ubuntu Linux 13.04(x86 arch), with GCC 4.7.1 installed. Tinydir is used for file management. The code is C++11 all through, therefore remember to turn the appropriate "switches on" before compilation.

Reports and Contribution
========================
Contact me via my email address: ogunyinkajoshua@yahoo.com

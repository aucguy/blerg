# The Blerg Programming Language
## Building
Currently only bash shells and the GNU compiler are supported. Python 3
is required to run the build script.

Run `python3 tools.py build` to build the executable. The executable is found
at `build/blerg`. `python3` may be `python` on your system.

Run `python3 tools.py clean` to clean the build files.

Run `python3 tools.py test` to run the tests.

Run `python3 tools.py valgrind` to run the tests using valgrind to check memory
violations. Obviously requires valgrind to be installed. On windows one may run
memcheck.bat in the windows cmd if they have WSL with valgrind (see below) installed.

##Installing Valgrind on windows
You may want to check for memory leaks using valgrind on a windows machine.
However, valgrind requires a linux environment to run. This requires one to
install the Windows Subsystem for Linux (WSL) which allows one to run linux
binaries (including valgrind) in a bash shell.

1. run `Enable-WindowsOptionalFeature -Online -FeatureName Microsoft-Windows-Subsystem-Linux`
in powershell with administrator privileges. Restart your computer.
2. visit [https://aka.ms/wslstore](https://aka.ms/wslstore) and install Ubuntu
or your preferred linux distro.
3. search for Ubuntu in the start menu and open it. Let the distro install and
then enter in a username and password as prompted.
4. search for bash in the start menu and open it.
5. run `sudo apt-get update` in bash to update the package list
6. run `sudo apt-get install build-essentials` in bash to get the GNU compiler
and other useful tools.
7. run `sudo apt-get install valgrind` in bash

From now on you can open up bash, go to the directory where you cloned the
repository and run `python3 tools.py valgrind` and use valgrind on a windows
machine.
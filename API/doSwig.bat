<<<<<<< HEAD
swig -python tinkercell.i
move /Y tinkercell_wrap.c ..\python
move /Y tinkercell.py ..\python
swig -octave -c++ tinkercell.i
move tinkercell_wrap.cxx ../octave/tinkercell_wrap.cpp
swig -perl tinkercell.i
move /Y tinkercell_wrap.c ..\perl
move /Y tinkercell.pm ..\perl
swig -ruby tinkercell.i
move /Y tinkercell_wrap.c ..\ruby
swig -r tinkercell.i
move /Y tinkercell_wrap.c ..\R
move /Y tinkercell.R ..\R
swig -java tinkercell.i
move /Y tinkercell_wrap.c ..\java
move /Y *.java ..\java
javac ..\java\*.java
=======
swig -python tinkercell.i
move /Y tinkercell_wrap.c ..\python
move /Y tinkercell.py ..\python
swig -octave -c++ tinkercell.i
move tinkercell_wrap.cxx ../octave/tinkercell_wrap.cpp
swig -perl tinkercell.i
move /Y tinkercell_wrap.c ..\perl
move /Y tinkercell.pm ..\perl
swig -ruby tinkercell.i
move /Y tinkercell_wrap.c ..\ruby
swig -r tinkercell.i
move /Y tinkercell_wrap.c ..\R
move /Y tinkercell.R ..\R
swig -java tinkercell.i
move /Y tinkercell_wrap.c ..\java
move /Y *.java ..\java
javac ..\java\*.java
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea

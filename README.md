# Tinkercell_new
How to Build
-You can build the project by steps stated in following link: https://docs.google.com/document/pub?id=1THLfivuMpA7W3PKtlHgiO_laJ8XS9XLnhV03U6wgm-A
-libSBOLc integration issues
--undefined function error solved when declare #LIBXML_STATIC in ModuleTools/sbolTools.h
-Mostly the project was built by setting debug mode by put “DEBUG” for BUILD_TYPE during CMake Build.
-I suggest to export the path of related libraries(QT, compilers such as MinGW) in environment variable(PATH for windows). Then, CMake will automatically locate libraries when you build the source.
-execute mingw32-make package in the BUILD directory
-After the build
--Copy ./BasicTools/NodesTree.nt and ./BasicTools/ConnectionsTree.nt into ./BUILD/bin folder
--Copy ./Graphics folder into ./BUILD/bin folder

Current Status
-Tinkercell is able to import and export SBOL document. Moreover, symbols in SBOL window can be exported to other windows, and add the components to other models for simulation.
-The current version of Tinkercell uses SBOL Visual v. 1.0 for the icon. Also, it uses libSBOLc for internal SBOL Document manipulation.
-The target SBOL version that editor complies is SBOL v. 1.1. The current version of Tinkercell doesn’t support further version of SBOL.

Build Environment
-Windows7, QT 4.8.6, Code::Block 13.12, mingw32-base 2013072200, mingw32-gcc-g++ 4.8.1-4
-WIndows 8.1, QT 4.8.6, Code::Block 13.12, mingw32-base 2013072200, mingw32-gcc-g++ 4.8.1-4

Minor Todos
-Some icons should be changed
--To change icons, you first need to code the icon in compliance with SBML Render Extension
--Then, you need to add the xml file in .\Core\Tinkercell.qrc
---ex)<file alias="DNATool.xml">../icons/DNATool.xml</file>
-Change SBOL export Icon
--ModuleTools\SBOLTool.cpp:177 - 190
-Change “Move SBOL icon to other documents” Icon
--ModuleTools\SBOLConverterTool.cpp:217-229
-Fixing SetPos problems when importing SBOL document and render it in network windows
--.\ModuleTools\SBOLTool.cpp:1267-1323

Major Todos
-Make binary release from the current source version
-Current build is able to make the install file by following original Tinkercell build manual(https://docs.google.com/document/pub?id=1THLfivuMpA7W3PKtlHgiO_laJ8XS9XLnhV03U6wgm-A, with following step 12(a) for windows) but the following error popped up. I think this caused by introducing libSBOLc into the previous Tinkercell. Some of the reference documents suggested that it happens when the linking library is formed without valid signal.

Other suggestion
-Change libSBOLc’s build path so that the project can use libSBOLc as submodule, directly.

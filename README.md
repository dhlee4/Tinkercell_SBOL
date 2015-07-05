# Tinkercell_new

<div>How to Build</div>
<ul>
<li>You can build the project by steps stated in following link: <a href="https://docs.google.com/document/pub?id=1THLfivuMpA7W3PKtlHgiO_laJ8XS9XLnhV03U6wgm-A">https://docs.google.com/document/pub?id=1THLfivuMpA7W3PKtlHgiO_laJ8XS9XLnhV03U6wgm-A</a></li>
<li>libSBOLc integration issues
<ul>
<li>undefined function error solved when declare #LIBXML_STATIC in ModuleTools/sbolTools.h</li>
</ul>
</li>
<li>Mostly the project was built by setting debug mode by put “DEBUG” for BUILD_TYPE during CMake Build.</li>
<li>I suggest to export the path of related libraries(QT, compilers such as MinGW) in environment variable(PATH for windows). Then, CMake will automatically locate libraries when you build the source.</li>
<li>execute mingw32-make package in the BUILD directory</li>
<li>After the build
<ul>
<li>Copy ./BasicTools/NodesTree.nt and ./BasicTools/ConnectionsTree.nt into ./BUILD/bin folder</li>
<li>Copy ./Graphics folder into ./BUILD/bin folder</li>
</ul>
</li>
</ul>
<div><br/></div>
<div>Current Status</div>
<ul>
<li>Tinkercell is able to import and export SBOL document. Moreover, symbols in SBOL window can be exported to other windows, and add the components to other models for simulation.</li>
<li>The current version of Tinkercell uses SBOL Visual v. 1.0 for the icon. Also, it uses libSBOLc for internal SBOL Document manipulation.</li>
<li>The target SBOL version that editor complies is SBOL v. 1.1. The current version of Tinkercell doesn’t support further version of SBOL.</li>
</ul>
<div><br/></div>
<div>Build Environment</div>
<ul>
<li>Windows7, QT 4.8.6, Code::Block 13.12, mingw32-base 2013072200, mingw32-gcc-g++ 4.8.1-4</li>
<li>WIndows 8.1, QT 4.8.6, Code::Block 13.12, mingw32-base 2013072200, mingw32-gcc-g++ 4.8.1-4</li>
</ul>
<div><br/></div>
<div>Minor Todos</div>
<ul>
<li>Some icons should be changed
<ul>
<li>To change icons, you first need to code the icon in compliance with SBML Render Extension</li>
<li>Then, you need to add the xml file in .\Core\Tinkercell.qrc
<ul>
<li>ex)&lt;file alias="DNATool.xml"&gt;../icons/DNATool.xml&lt;/file&gt;</li>
</ul>
</li>
</ul>
</li>
<li>Change SBOL export Icon
<ul>
<li>ModuleTools\SBOLTool.cpp:177 - 190</li>
</ul>
</li>
<li>Change “Move SBOL icon to other documents” Icon
<ul>
<li>ModuleTools\SBOLConverterTool.cpp:217-229</li>
</ul>
</li>
<li>Fixing SetPos problems when importing SBOL document and render it in network windows
<ul>
<li>.\ModuleTools\SBOLTool.cpp:1267-1323</li>
</ul>
</li>
</ul>
<div><br/></div>
<div>Major Todos</div>
<ul>
<li>Make binary release from the current source version</li>
<li>Current build is able to make the install file by following original Tinkercell build manual(<a href="https://docs.google.com/document/pub?id=1THLfivuMpA7W3PKtlHgiO_laJ8XS9XLnhV03U6wgm-A">https://docs.google.com/document/pub?id=1THLfivuMpA7W3PKtlHgiO_laJ8XS9XLnhV03U6wgm-A</a>, with following step 12(a) for windows) but the following error popped up. I think this caused by introducing libSBOLc into the previous Tinkercell. Some of the reference documents suggested that it happens when the linking library is formed without valid signal.</li>
</ul>
<div><br/></div>
<div>Other suggestion</div>
<ul>
<li>Change libSBOLc’s build path so that the project can use libSBOLc as submodule, directly.</li>
</ul>


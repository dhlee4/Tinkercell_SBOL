/**
 * @file    util.h
 * @brief   Supporting functions for example code
 * @author  Ben Bornstein
 *
<<<<<<< HEAD
 * $Id: util.h 2657 2011-10-05 21:28:22Z dchandran1 $
 * $HeadURL: svn://svn.code.sf.net/p/tinkercell/code/trunk/ExternalCode/sbml/examples/c++/util.h $
=======
 * $Id$
 * $HeadURL$
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
 *
 * This file is part of libSBML.  Please visit http://sbml.org for more
 * information about SBML, and the latest version of libSBML.
 */

#include <sbml/common/extern.h>


BEGIN_C_DECLS

/**
 * @return the number of milliseconds elapsed since the Epoch.
 */
unsigned long long
getCurrentMillis (void);

/**
 * @return the size (in bytes) of the given filename.
 */
unsigned long
getFileSize (const char* filename);

END_C_DECLS

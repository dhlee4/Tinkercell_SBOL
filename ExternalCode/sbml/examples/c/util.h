/**
 * @file    util.c
 * @brief   Supporting functions for example code
 * @author  Ben Bornstein
 * @author  Michael Hucka
 *
<<<<<<< HEAD
 * $Id: util.h 2657 2011-10-05 21:28:22Z dchandran1 $
 * $HeadURL: svn://svn.code.sf.net/p/tinkercell/code/trunk/ExternalCode/sbml/examples/c/util.h $
=======
 * $Id$
 * $HeadURL$
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
 *
 * This file is part of libSBML.  Please visit http://sbml.org for more
 * information about SBML, and the latest version of libSBML.
 */


/**
 * @return the number of milliseconds elapsed since the Epoch.
 */
unsigned long long
getCurrentMillis (void);

/**
 * @return the size (in bytes) of the given filename.
 */
unsigned long
getFileSize (const char *filename);

/**
 * Removes whitespace from both ends of the given string.  The string
 * is modified in-place.  This function returns a pointer to the (same)
 * string buffer.
 */
char *
trim_whitespace (char *s);

/**
 * The function get_line reads a line from a file (in this case "stdin" and
 * returns it as a string.  It is taken from the utilities library of the
 * VIENNA RNA PACKAGE ( http://www.tbi.univie.ac.at/~ivo/RNA/ )
 */
char*
get_line (FILE *fp);

/**
 * @file    LayoutAnnotation.h
 * @brief   Layout annotation I/O
 * @author  Ralph Gauges
 *
<<<<<<< HEAD
 * $Id: LayoutAnnotation.h 2657 2011-10-05 21:28:22Z dchandran1 $
 * $HeadURL: svn://svn.code.sf.net/p/tinkercell/code/trunk/ExternalCode/sbml/src/sbml/packages/layout/util/LayoutAnnotation.h $
=======
 * $Id$
 * $HeadURL$
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea
 *
 * <!--------------------------------------------------------------------------
 * This file is part of libSBML.  Please visit http://sbml.org for more
 * information about SBML, and the latest version of libSBML.
 *
 * Copyright (C) 2009-2011 jointly by the following organizations: 
 *     1. California Institute of Technology, Pasadena, CA, USA
 *     2. EMBL European Bioinformatics Institute (EBML-EBI), Hinxton, UK
 *  
 * Copyright (C) 2006-2008 by the California Institute of Technology,
 *     Pasadena, CA, USA 
 *  
 * Copyright (C) 2002-2005 jointly by the following organizations: 
 *     1. California Institute of Technology, Pasadena, CA, USA
 *     2. Japan Science and Technology Agency, Japan
 * 
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.  A copy of the license agreement is provided
 * in the file named "LICENSE.txt" included with this software distribution
 * and also available online as http://sbml.org/software/libsbml/license.html
 * ---------------------------------------------------------------------- -->*/

#ifndef LayoutAnnotation_h
#define LayoutAnnotation_h


#include <sbml/common/extern.h>
#include <sbml/common/sbmlfwd.h>

#include <sbml/xml/XMLAttributes.h>

#include <sbml/SpeciesReference.h>
#include <sbml/packages/layout/sbml/Layout.h>

#ifdef __cplusplus

#include <limits>
#include <iomanip>
#include <string>
#include <sstream>

#include <cstdlib>

LIBSBML_CPP_NAMESPACE_BEGIN

/**
 * takes an annotation that has been read into the model
 * identifies the RDF elements
 * and creates a List of Layouts from the annotation
 */
LIBSBML_EXTERN
void parseLayoutAnnotation(XMLNode * annotation, ListOfLayouts& layouts);

/**
 * Takes an XMLNode and tries to find the layout annotation node and deletes it if it was found.
 */
LIBSBML_EXTERN
XMLNode* deleteLayoutAnnotation(XMLNode* pAnnotation);

/**
 * Creates an XMLNode that represents the layouts of the model from the given Model object.
 */
 LIBSBML_EXTERN
 XMLNode* parseLayouts(const Model* pModel);
 
/**
 * takes an annotation that has been read into the species reference
 * identifies the id elements and set the id of the species reference
 */
LIBSBML_EXTERN
void 
parseSpeciesReferenceAnnotation(XMLNode * annotation, SimpleSpeciesReference& sr);

/**
 * Takes an XMLNode and tries to find the layoutId annotation node and deletes it if it was found.
 */
LIBSBML_EXTERN
XMLNode* deleteLayoutIdAnnotation(XMLNode* pAnnotation);

/**
 * Creates an XMLNode that represents the layoutId annotation of the species reference from the given SpeciesReference object.
 */
LIBSBML_EXTERN
XMLNode* parseLayoutId(const SimpleSpeciesReference* sr);


LIBSBML_CPP_NAMESPACE_END

#endif  /* __cplusplus */
#endif  /** LayoutAnnotation_h **/

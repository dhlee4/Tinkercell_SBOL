/**
 * @cond doxygen-libsbml-internal
 *
 * @file    DuplicateTopLevelAnnotation.h
 * @brief   Checks for duplicate top level annotations
 * @author  Sarah Keating
 *
<<<<<<< HEAD
 * $Id: DuplicateTopLevelAnnotation.h 2657 2011-10-05 21:28:22Z dchandran1 $
 * $HeadURL: svn://svn.code.sf.net/p/tinkercell/code/trunk/ExternalCode/sbml/src/sbml/validator/constraints/DuplicateTopLevelAnnotation.h $
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

#ifndef DuplicateTopLevelAnnotation_h
#define DuplicateTopLevelAnnotation_h


#ifdef __cplusplus

#include <string>

#include <sbml/validator/VConstraint.h>

#include "IdList.h"

LIBSBML_CPP_NAMESPACE_BEGIN

class DuplicateTopLevelAnnotation: public TConstraint<Model>
{
public:

  /**
   * Creates a new Constraint with the given constraint id.
   */
  DuplicateTopLevelAnnotation (unsigned int id, Validator& v);

  /**
   * Destroys this Constraint.
   */
  virtual ~DuplicateTopLevelAnnotation ();


protected:

  /**
   * Checks that <ci> element after an apply is already listed as a FunctionDefinition.
   */
  virtual void check_ (const Model& m, const Model& object);

  /**
   * Checks that <ci> element after an apply is already listed as a FunctionDefinition.
   */
  void checkAnnotation(const SBase& object);

  /**
   * Logs a message about an undefined <ci> element in the given
   * FunctionDefinition.
   */
  void logDuplicate (const std::string name, const SBase& object);

  IdList mNamespaces;

};

LIBSBML_CPP_NAMESPACE_END
#endif  /* __cplusplus */
#endif  /* DuplicateTopLevelAnnotation_h */

/** @endcond */

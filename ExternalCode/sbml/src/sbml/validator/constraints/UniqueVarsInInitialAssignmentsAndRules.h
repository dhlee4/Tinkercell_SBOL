/**
 * @cond doxygen-libsbml-internal
 *
 * @file    UniqueVarsInInitialAssignmentsAndRules.h
 * @brief   Ensures unique variables assigned by rules and events
 * @author  Sarah Keating
 *
<<<<<<< HEAD
 * $Id: UniqueVarsInInitialAssignmentsAndRules.h 2657 2011-10-05 21:28:22Z dchandran1 $
 * $HeadURL: svn://svn.code.sf.net/p/tinkercell/code/trunk/ExternalCode/sbml/src/sbml/validator/constraints/UniqueVarsInInitialAssignmentsAndRules.h $
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

#ifndef UniqueVarsInInitialAssignmentsAndRules_h
#define UniqueVarsInInitialAssignmentsAndRules_h


#ifdef __cplusplus

#include <string>

#include "UniqueIdBase.h"

LIBSBML_CPP_NAMESPACE_BEGIN

class UniqueVarsInInitialAssignmentsAndRules: public UniqueIdBase
{
public:

  /**
   * Creates a new Constraint with the given constraint id.
   */
  UniqueVarsInInitialAssignmentsAndRules (unsigned int id, Validator& v);

  /**
   * Destroys this Constraint.
   */
  virtual ~UniqueVarsInInitialAssignmentsAndRules ();


protected:

  /**
   * @return the preamble to use when logging constraint violations.
   */
  virtual const char* getPreamble ();

  /**
   * Checks that all ids on the following Model objects are unique:
   * FunctionDefinitions, Species, Compartments, global Parameters,
   * Reactions, and Events.
   */
  virtual void doCheck (const Model& m);
};

LIBSBML_CPP_NAMESPACE_END

#endif  /* __cplusplus */
#endif  /* UniqueVarsInInitialAssignmentsAndRules_h */

/** @endcond */

/**
 * @cond doxygen-libsbml-internal
 *
 * @file    ConsistencyValidator.h
 * @brief   Performs consistency checks on an SBML model
 * @author  Ben Bornstein
 *
 * $Id: ConsistencyValidator.h 2657 2011-10-05 21:28:22Z dchandran1 $
 * $HeadURL: svn://svn.code.sf.net/p/tinkercell/code/trunk/ExternalCode/sbml/src/sbml/validator/ConsistencyValidator.h $
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
 * in the file named "LICENSE.txt" included with this software distribution and
 * also available online as http://sbml.org/software/libsbml/license.html
 * ---------------------------------------------------------------------- -->*/

#ifndef ConsistencyValidator_h
#define ConsistencyValidator_h


#ifdef __cplusplus


#include <sbml/validator/Validator.h>
#include <sbml/SBMLError.h>

LIBSBML_CPP_NAMESPACE_BEGIN

class ConsistencyValidator: public Validator
{
public:

  ConsistencyValidator () : Validator( LIBSBML_CAT_GENERAL_CONSISTENCY ) { }

  virtual ~ConsistencyValidator () { }

  /**
   * Initializes this Validator with a set of Constraints.
   */
  virtual void init ();
};

LIBSBML_CPP_NAMESPACE_END

#endif  /* __cplusplus */
#endif  /* ConsistencyValidator_h */


/** @endcond */

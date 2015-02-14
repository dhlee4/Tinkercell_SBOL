<<<<<<< HEAD
/**
 * @file SignatureElement.h
 * @brief represents an argument in a signature
 * @author SBW Development Group <sysbio-team@caltech.edu>
 *
 * Organization: Caltech ERATO Kitano Systems Biology Project
 *
 * Created: @date 18th June 2001
 * $Id: SignatureElement.h,v 1.1 2005/12/15 01:06:44 fbergmann Exp $
 * $Source: /cvsroot/sbw/core/include/SBW/SignatureElement.h,v $
 */

/*
** Copyright 2001 California Institute of Technology and
** Japan Science and Technology Corporation.
** 
** This library is free software; you can redistribute it and/or modify it
** under the terms of the GNU Lesser General Public License as published
** by the Free Software Foundation; either version 2.1 of the License, or
** any later version.
** 
** This library is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY, WITHOUT EVEN THE IMPLIED WARRANTY OF
** MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  The software and
** documentation provided hereunder is on an "as is" basis, and the
** California Institute of Technology and Japan Science and Technology
** Corporation have no obligations to provide maintenance, support,
** updates, enhancements or modifications.  In no event shall the
** California Institute of Technology or the Japan Science and Technology
** Corporation be liable to any party for direct, indirect, special,
** incidental or consequential damages, including lost profits, arising
** out of the use of this software and its documentation, even if the
** California Institute of Technology and/or Japan Science and Technology
** Corporation have been advised of the possibility of such damage.  See
** the GNU Lesser General Public License for more details.
** 
** You should have received a copy of the GNU Lesser General Public License
** along with this library; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
**
** The original code contained here was initially developed by:
**
**     Andrew Finney, Herbert Sauro, Michael Hucka, Hamid Bolouri
**     The Systems Biology Workbench Development Group
**     ERATO Kitano Systems Biology Project
**     Control and Dynamical Systems, MC 107-81
**     California Institute of Technology
**     Pasadena, CA, 91125, USA
**
**     http://www.cds.caltech.edu/erato
**     mailto:sysbio-team@caltech.edu
**
** Contributor(s):
**
*/

/// MSVC generated include loop block
#if !defined(AFX_SignatureElement_H__E7246DD7_B41C_4AAE_ADE2_D796A88AE63B__INCLUDED_)
#define AFX_SignatureElement_H__E7246DD7_B41C_4AAE_ADE2_D796A88AE63B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include "SBWHandle.h"
#include "SBWObject.h"

namespace SystemsBiologyWorkbench
{

struct sbwSignatureType;
class SignatureType ;

/// underlying implementation of SignatureElement - represents an argument in a signature
struct sbwSignatureElement : public SBWObject
{
public:
	sbwSignatureElement(const char *&);
	sbwSignatureElement(SBWHandle<sbwSignatureType>);

	/**
	 * return the name of the argument
	 * @return the name of the argument
	 */
	std::string getName() { return name ; }

	SBW_API void getType(SignatureType &x);

private:
	/// the name of the argument
	std::string name ;

	/// the type of the argument
	SBWHandle<sbwSignatureType> type ;
};

/// represents an argument in a signature
class SignatureElement : public SBWHandle<sbwSignatureElement>
{
public:
	/// creates an empty argument structure
	SignatureElement() {} ;
	SBW_API SignatureElement(const char *&);
	SBW_API SignatureElement(SBWHandle<sbwSignatureType>);

	/**
	 * returns the name of the argument
	 * @return the name of the argument
	 */
	std::string getName() { return p->getName() ; }

	/**
	 * fetch the type of the argument
	 * @param x set ot the type of the argument
	 */
	void getType(SignatureType  &x) const { p->getType(x) ; }

	/**
	 * returns the underlying implementation object
	 * @return the underlying implementation object
	 */
	sbwSignatureElement *getImpl() { return p; }
};

} // SystemsBiologyWorkbench
#endif // !defined(AFX_SignatureElement_H__E7246DD7_B41C_4AAE_ADE2_D796A88AE63B__INCLUDED_)
=======
/**
 * @file SignatureElement.h
 * @brief represents an argument in a signature
 * @author SBW Development Group <sysbio-team@caltech.edu>
 *
 * Organization: Caltech ERATO Kitano Systems Biology Project
 *
 * Created: @date 18th June 2001
 * $Id: SignatureElement.h,v 1.1 2005/12/15 01:06:44 fbergmann Exp $
 * $Source: /cvsroot/sbw/core/include/SBW/SignatureElement.h,v $
 */

/*
** Copyright 2001 California Institute of Technology and
** Japan Science and Technology Corporation.
** 
** This library is free software; you can redistribute it and/or modify it
** under the terms of the GNU Lesser General Public License as published
** by the Free Software Foundation; either version 2.1 of the License, or
** any later version.
** 
** This library is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY, WITHOUT EVEN THE IMPLIED WARRANTY OF
** MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  The software and
** documentation provided hereunder is on an "as is" basis, and the
** California Institute of Technology and Japan Science and Technology
** Corporation have no obligations to provide maintenance, support,
** updates, enhancements or modifications.  In no event shall the
** California Institute of Technology or the Japan Science and Technology
** Corporation be liable to any party for direct, indirect, special,
** incidental or consequential damages, including lost profits, arising
** out of the use of this software and its documentation, even if the
** California Institute of Technology and/or Japan Science and Technology
** Corporation have been advised of the possibility of such damage.  See
** the GNU Lesser General Public License for more details.
** 
** You should have received a copy of the GNU Lesser General Public License
** along with this library; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
**
** The original code contained here was initially developed by:
**
**     Andrew Finney, Herbert Sauro, Michael Hucka, Hamid Bolouri
**     The Systems Biology Workbench Development Group
**     ERATO Kitano Systems Biology Project
**     Control and Dynamical Systems, MC 107-81
**     California Institute of Technology
**     Pasadena, CA, 91125, USA
**
**     http://www.cds.caltech.edu/erato
**     mailto:sysbio-team@caltech.edu
**
** Contributor(s):
**
*/

/// MSVC generated include loop block
#if !defined(AFX_SignatureElement_H__E7246DD7_B41C_4AAE_ADE2_D796A88AE63B__INCLUDED_)
#define AFX_SignatureElement_H__E7246DD7_B41C_4AAE_ADE2_D796A88AE63B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include "SBWHandle.h"
#include "SBWObject.h"

namespace SystemsBiologyWorkbench
{

struct sbwSignatureType;
class SignatureType ;

/// underlying implementation of SignatureElement - represents an argument in a signature
struct sbwSignatureElement : public SBWObject
{
public:
	sbwSignatureElement(const char *&);
	sbwSignatureElement(SBWHandle<sbwSignatureType>);

	/**
	 * return the name of the argument
	 * @return the name of the argument
	 */
	std::string getName() { return name ; }

	SBW_API void getType(SignatureType &x);

private:
	/// the name of the argument
	std::string name ;

	/// the type of the argument
	SBWHandle<sbwSignatureType> type ;
};

/// represents an argument in a signature
class SignatureElement : public SBWHandle<sbwSignatureElement>
{
public:
	/// creates an empty argument structure
	SignatureElement() {} ;
	SBW_API SignatureElement(const char *&);
	SBW_API SignatureElement(SBWHandle<sbwSignatureType>);

	/**
	 * returns the name of the argument
	 * @return the name of the argument
	 */
	std::string getName() { return p->getName() ; }

	/**
	 * fetch the type of the argument
	 * @param x set ot the type of the argument
	 */
	void getType(SignatureType  &x) const { p->getType(x) ; }

	/**
	 * returns the underlying implementation object
	 * @return the underlying implementation object
	 */
	sbwSignatureElement *getImpl() { return p; }
};

} // SystemsBiologyWorkbench
#endif // !defined(AFX_SignatureElement_H__E7246DD7_B41C_4AAE_ADE2_D796A88AE63B__INCLUDED_)
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea

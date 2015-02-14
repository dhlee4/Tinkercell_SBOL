<<<<<<< HEAD
/**
 * @file Method.h
 * @brief  represents a method on potentially another module
 * @author SBW Development Group <sysbio-team@caltech.edu>
 *
 * Organization: Caltech ERATO Kitano Systems Biology Project
 *
 * Created: @date 18th June 2001
 * $Id: Method.h,v 1.1 2005/12/15 01:06:44 fbergmann Exp $
 * $Source: /cvsroot/sbw/core/include/SBW/Method.h,v $
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
*/// Method.h: interface for the Method class.
//
//////////////////////////////////////////////////////////////////////

/// MSVC generated include loop block
#if !defined(AFX_METHOD_H__5068A13F_2315_43CE_A08E_02CEA2676DC5__INCLUDED_)
#define AFX_METHOD_H__5068A13F_2315_43CE_A08E_02CEA2676DC5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "DataBlockReaderClass.h"
#include "DataBlockWriterClass.h"
#include "Signature.h"

namespace SystemsBiologyWorkbench
{

class Service ;

/// represents a method on a module instance
class ServiceMethod  
{
public:
	ServiceMethod();
	std::string getName();
	std::string getSignatureString();
	std::string getHelp();

	DataBlockReader call(DataBlockWriter args);
	void send(DataBlockWriter args);

	SBW_API void getService(Service &);
	Signature getSignature();

private:
	ServiceMethod(Integer moduleId, Integer serviceId, Integer methodId);

	/// module instance numeric identifier
	Integer moduleId;

	/// service numeric identifier
	Integer serviceId;

	/// method numeric identifier
	Integer methodId;

	friend class Service;
	friend class SBWLowLevel;
};

} // SystemsBiologyWorkbench

#endif // !defined(AFX_METHOD_H__5068A13F_2315_43CE_A08E_02CEA2676DC5__INCLUDED_)
=======
/**
 * @file Method.h
 * @brief  represents a method on potentially another module
 * @author SBW Development Group <sysbio-team@caltech.edu>
 *
 * Organization: Caltech ERATO Kitano Systems Biology Project
 *
 * Created: @date 18th June 2001
 * $Id: Method.h,v 1.1 2005/12/15 01:06:44 fbergmann Exp $
 * $Source: /cvsroot/sbw/core/include/SBW/Method.h,v $
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
*/// Method.h: interface for the Method class.
//
//////////////////////////////////////////////////////////////////////

/// MSVC generated include loop block
#if !defined(AFX_METHOD_H__5068A13F_2315_43CE_A08E_02CEA2676DC5__INCLUDED_)
#define AFX_METHOD_H__5068A13F_2315_43CE_A08E_02CEA2676DC5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "DataBlockReaderClass.h"
#include "DataBlockWriterClass.h"
#include "Signature.h"

namespace SystemsBiologyWorkbench
{

class Service ;

/// represents a method on a module instance
class ServiceMethod  
{
public:
	ServiceMethod();
	std::string getName();
	std::string getSignatureString();
	std::string getHelp();

	DataBlockReader call(DataBlockWriter args);
	void send(DataBlockWriter args);

	SBW_API void getService(Service &);
	Signature getSignature();

private:
	ServiceMethod(Integer moduleId, Integer serviceId, Integer methodId);

	/// module instance numeric identifier
	Integer moduleId;

	/// service numeric identifier
	Integer serviceId;

	/// method numeric identifier
	Integer methodId;

	friend class Service;
	friend class SBWLowLevel;
};

} // SystemsBiologyWorkbench

#endif // !defined(AFX_METHOD_H__5068A13F_2315_43CE_A08E_02CEA2676DC5__INCLUDED_)
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea

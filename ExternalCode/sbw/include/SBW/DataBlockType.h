/**
 * @file DataBlockType.h
 * @brief  enumeration of types for C
 * @author SBW Development Group <sysbio-team@caltech.edu>
 *
 * Organization: Caltech ERATO Kitano Systems Biology Project
 *
 * Created: @date 18th June 2001
 * $Id: DataBlockType.h,v 1.1 2005/12/15 01:06:44 fbergmann Exp $
 * $Source: /cvsroot/sbw/core/include/SBW/DataBlockType.h,v $
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

/// include loop block
#ifndef DATABLOCKTYPE
#define DATABLOCKTYPE

/**
 * Exception types used in C API.
 * uses standard encoding of exceptions used in both in the API and in exception messages.
 */
typedef enum sbwExceptionCode
{
	SBWApplicationExceptionCode = 0, /**< generated by a module ie not raised by the SBW infastructure */
    SBWRawExceptionCode = 1, /**< generated by a OS or unhandled exception */
    SBWCommunicationExceptionCode = 2, /**< communication between modules has been disrupted */
    SBWModuleStartExceptionCode = 3, /**< unable to start module when a new module instance was required */
    SBWTypeMismatchExceptionCode = 4, /**< the data contained in a datablock doesn't correspond to the requested type */
    SBWIncomptatibleMethodSignatureExceptionCode = 5, /**< two method signatures do not match */
    SBWModuleIdSyntaxExceptionCode = 6, /**< the syntax of a module instance identifier string is incorrect */
    SBWIncorrectCategorySyntaxExceptionCode = 7, /**< the syntax of a service category string is incorrect */
    SBWServiceNotFoundExceptionCode = 9, /**< the requested service doesn't exist */ 
    SBWMethodTypeNotBlockTypeExceptionCode = 10, /**< thrown during communications if a supplied class uses types which are not data block types (not raised in C++ library) */
    SBWMethodAmbiguousExceptionCode = 11, /**< the given signature matches more than one method on a service */
    SBWUnsupportedObjectTypeExceptionCode = 12, /**< the given valid type is not supported by a client library */
    SBWMethodNotFoundExceptionCode = 13, /**< the given signature or name doesn't match any method on a service */
	SBWSignatureSyntaxExceptionCode = 14, /**< the syntax of the given signature is incorrect */
	SBWModuleDefinitionExceptionCode = 15, /**< exception thrown when an attempt to define a new module fails */
	SBWModuleNotFoundExceptionCode = 16, /**< the requested module doesn't exist */
	SBWBrokerStartExceptionCode = 17, /**< the requested module doesn't exist */
} SBWExceptionType ;

/**
 * Datatypes used in C API.
 * uses standard encoding of datatypes used in both the API and in datablocks.
 */
typedef enum sbwdataBlockType
{
	SBWByteType = 0 , /**< C API type - unsigned char */
	SBWIntegerType = 1, /**< C API type - SBWInteger */ 
	SBWDoubleType = 2, /**< C API type - SBWDouble */
	SBWBooleanType = 3, /**< C API type - SBWBoolean */
	SBWStringType = 4, /**< C API type - char * */
	SBWArrayType = 5, /**< C API type - <T>* or <T>** */
	SBWListType = 6, /**< C API type - SBWDataBlockReader * or SBWDataBlockWriter * */
	SBWVoidType = 7, /**< placeholder when no value is returned from a method */
	SBWComplexType = 8, /**< C API type - SBWComplex */ 
	SBWTerminateType = -1, /**< value of last byte in a datablock or message */
	SBWErrorType = -2 /**< placeholder when error has occured in an API function */
} SBWDataBlockType;

/**
 * The modes in which a given module/application can operate with respect to SBW
 */
typedef enum sbwmoduleMode
{
	SBWNormalMode = 0, /**< either not connected or only consuming services */
	SBWModuleMode = 1, /**< providing services */
	SBWRegisterMode = 2 /**< registering services */
} SBWModuleModeType;

/**
 * The ways in which a given module can be managed by the broker.
 * Uses standard encoding used by both by the API and the broker interface.
 */
typedef enum sbwmoduleManagementType
{
	SBW_UniqueModule = 0, /**< broker only creates one instance of the module and refers all requests to that instance.*/
	SBW_SelfManagedModule = 1 /**< broker creates a new instance for each module instance request */
	//SBW_SBWManagedModule = 2 /**< not currently used - as for self managed but will shutdown module if other module instances no longer reference the module instance. */
} SBWModuleManagementType;

#endif // DATABLOCKTYPE

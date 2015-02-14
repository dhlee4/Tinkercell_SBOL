<<<<<<< HEAD
#ifndef qwt3d_mapping_h__2004_03_05_13_51_begin_guarded_code
#define qwt3d_mapping_h__2004_03_05_13_51_begin_guarded_code

#include <qstring.h>
#include "qwt3d_global.h"
#include "qwt3d_types.h"

namespace Qwt3D
{

//! Abstract base class for general mappings
/**

*/
class QWT3D_EXPORT Mapping
{

public:
	
  virtual ~Mapping(){} //!< Destructor.
	virtual QString name() const { return QString(""); } //!< Descriptive String.
};


} // ns

#endif /* include guarded */
=======
#ifndef qwt3d_mapping_h__2004_03_05_13_51_begin_guarded_code
#define qwt3d_mapping_h__2004_03_05_13_51_begin_guarded_code

#include <qstring.h>
#include "qwt3d_global.h"
#include "qwt3d_types.h"

namespace Qwt3D
{

//! Abstract base class for general mappings
/**

*/
class QWT3D_EXPORT Mapping
{

public:
	
  virtual ~Mapping(){} //!< Destructor.
	virtual QString name() const { return QString(""); } //!< Descriptive String.
};


} // ns

#endif /* include guarded */
>>>>>>> 1905cfc9e294ef1fd9bc7c874b4a4e2af0fff3ea

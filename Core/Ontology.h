/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

This class is used to load and store a set of NodeFamily instances and ConnectionFamily
instances. The families are loaded from RDF files.

****************************************************************************/

#ifndef TINKERCELL_FAMILYONTOLOGY_H
#define TINKERCELL_FAMILYONTOLOGY_H

#include <QList>
#include <QStringList>
#include <QHash>

#ifndef TINKERCELLCOREEXPORT
#ifdef Q_WS_WIN
#   if defined(TC_EXPORTS) || defined(TinkerCellCore_EXPORTS)
#       define TINKERCELLCOREEXPORT __declspec(dllexport)
#   else
#       define TINKERCELLCOREEXPORT __declspec(dllimport)
#   endif
#else
#    define TINKERCELLCOREEXPORT
#endif
#endif

namespace Tinkercell
{
	class ItemFamily;
	class NodeFamily;
	class ConnectionFamily;
	typedef ItemFamily ParticipantRole;
	
	/*! \brief A set of node and connection families. All functions are static
	\ingroup core
	*/
	class TINKERCELLCOREEXPORT Ontology
	{
	public:
		/*! \brief list of families that are parents to any other family, e.g. the empty set or something like that. 
				This list should remain empty if such a concept does not make sense for the application
		*/
		static QStringList GLOBAL_PARENTS;
		/*! \brief list of families that are children to any other family, e.g. the empty set or something like that. 
				This list should remain empty if such a concept does not make sense for the application
		*/
		static QStringList GLOBAL_CHILDREN;
		/*! \brief get a family pointer given its name (not case-sensitive)
			\param QString& name of family
			\return NodeFamily* can be 0 if name does not exist
		*/
		static NodeFamily * nodeFamily(const QString&);
		/*! \brief get a family pointer given its name (not case-sensitive)
			\param QString& name of family
			\return ConnectionFamily* can be 0 if name does not exist
		*/	
		static ConnectionFamily * connectionFamily(const QString&);

		/*! \brief get a family pointer given its name (not case-sensitive)
			\param QString& name of family
			\return ParticipantRole* can be 0 if name does not exist
		*/	
		static ParticipantRole * participantRole(const QString&);

		/*! \brief add a new family to the ontology
			\param QString family name
			\param NodeFamily* new family
			\return bool did the insertion succeed
		*/	
		static bool insertNodeFamily(const QString &, NodeFamily * );
		/*! \brief add a new family to the ontology
			\param QString family name
			\param ConnectionFamily* new family
			\return bool did the insertion succeed
		*/	
		static bool insertConnectionFamily(const QString &, ConnectionFamily *);
		/*! \brief add a new participant type to the ontology
			\param QString role name
			\return bool did the insertion succeed
		*/	
		static bool insertParticipantRole(const QString &, ParticipantRole *);
		/*! \brief get list of all node families
			\param QList<NodeFamily*>
		*/	
		static QList<NodeFamily*> allNodeFamilies(); 
		/*! \brief get list of all connection families
			\param QList<ConnectionFamily*>
		*/	
		static QList<ConnectionFamily*> allConnectionFamilies();
		/*! \brief get list of all node family names
			\param QStringList
		*/	
		static QStringList allNodeFamilyNames(); 
		/*! \brief get list of all connection family names
			\param QStringList
		*/	
		static QStringList allConnectionFamilyNames();

		/*! \brief read RDF file and insert node families
			\param QString file name
			\param QString format, defaults to rdfxml
			\return QStringList new families
		*/	
		static QStringList readNodes(const QString& rdfFile, const QString& format=QString("rdfxml"));
		/*! \brief read RDF file and insert connection families
			\param QString file name
			\param QString format, defaults to rdfxml
			\return QStringList new families
		*/	
		static QStringList readConnections(const QString& rdfFile, const QString& format=QString("rdfxml"));

		/*! \brief delete family instances*/
		static void cleanup();
	private:
		/*! \brief maps name to node families*/
		static QHash<QString, NodeFamily*> nodeFamilies;
		/*! \brief maps name to connection families*/
		static QHash<QString, ConnectionFamily*> connectionFamilies;
		/*! \brief maps name to participant role families*/
		static QHash<QString, ParticipantRole*> participantRoleFamilies;
	};
}

#endif


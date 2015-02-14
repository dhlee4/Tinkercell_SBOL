#include "ItemFamily.h"
#include "Ontology.h"

#ifndef NOT_USING_RAPTOR
extern "C"
{
	#include "raptor.h"
}
#endif

namespace Tinkercell
{
		QStringList Ontology::GLOBAL_PARENTS;
		QStringList Ontology::GLOBAL_CHILDREN;

		NodeFamily * Ontology::nodeFamily(const QString& s0)
		{
			QString s = s0.toLower();
			if (nodeFamilies.contains(s))
				return nodeFamilies.value(s);
			else
				return 0;
		}

		ConnectionFamily  * Ontology::connectionFamily(const QString& s0)
		{
			QString s = s0.toLower();
			if (connectionFamilies.contains(s))
				return connectionFamilies.value(s);
			else
				return 0;
		}

		ParticipantRole * Ontology::participantRole(const QString& s0)
		{
			QString s = s0.toLower();
			if (participantRoleFamilies.contains(s))
				return participantRoleFamilies.value(s);
			else
				return 0;
		}

		bool Ontology::insertNodeFamily(const QString & s0, NodeFamily * ptr)
		{
			QString s = s0.toLower();
			if (!ptr || s.isEmpty() || nodeFamilies.contains(s)) return false;
			nodeFamilies.insert(s,ptr);
			if (s != ptr->name())
				nodeFamilies.insert(ptr->name(), ptr);
			return true;
		}

		bool Ontology::insertConnectionFamily(const QString & s0, ConnectionFamily * ptr)
		{
			QString s = s0.toLower();
			if (!ptr || s.isEmpty() || connectionFamilies.contains(s)) return false;
			connectionFamilies.insert(s,ptr);
			if (s != ptr->name())
				connectionFamilies.insert(ptr->name(), ptr);
			return true;
		}

		bool Ontology::insertParticipantRole(const QString & s0, ParticipantRole * ptr)
		{
			QString s = s0.toLower();
			if (!ptr || s.isEmpty() || participantRoleFamilies.contains(s)) return false;
			participantRoleFamilies.insert(s,ptr);
			if (s != ptr->name())
				participantRoleFamilies.insert(ptr->name(), ptr);
			return true;
		}

		QList<NodeFamily*> Ontology::allNodeFamilies()
		{
			QList<NodeFamily*> all = nodeFamilies.values(), unique;
			for (int i=0; i < all.size(); ++i)
				if (all[i] && !unique.contains(all[i]))
					unique += all[i];
			return unique;
		}

		QList<ConnectionFamily*> Ontology::allConnectionFamilies()
		{
			QList<ConnectionFamily*> all = connectionFamilies.values(), unique;
			for (int i=0; i < all.size(); ++i)
				if (all[i] && !unique.contains(all[i]))
					unique += all[i];
			return unique;
		}

		QStringList Ontology::allNodeFamilyNames()
		{
			QStringList keys(nodeFamilies.keys()), names;
			for (int i=0; i < keys.size(); ++i)
				if (nodeFamilies[ keys[i] ] && nodeFamilies[ keys[i] ]->name() == keys[i])
					names << keys[i];
			return names;
		}

		QStringList Ontology::allConnectionFamilyNames()
		{
			QStringList keys(connectionFamilies.keys()), names;
			for (int i=0; i < keys.size(); ++i)
				if (connectionFamilies[ keys[i] ] && connectionFamilies[ keys[i] ]->name() == keys[i])
					names << keys[i];
			return names;
		}

#ifndef NOT_USING_RAPTOR
		static QStringList lastReadFamilyNames;
		static void read_node_triple(void* user_data, raptor_statement* triple) 
		{
			QString s,p,o;
			NodeFamily *family1 = 0, *family2 = 0;

			if (triple->subject->type == RAPTOR_TERM_TYPE_URI)
				s = QObject::tr((char*)(raptor_uri_as_string(triple->subject->value.uri)));
			else
			if (triple->subject->type == RAPTOR_TERM_TYPE_LITERAL)
				s = QObject::tr((char*)(triple->subject->value.literal.string));

			if (triple->predicate->type == RAPTOR_TERM_TYPE_URI)
				p = QObject::tr((char*)(raptor_uri_as_string(triple->predicate->value.uri)));
			else
			if (triple->predicate->type == RAPTOR_TERM_TYPE_LITERAL)
				p = QObject::tr((char*)(triple->predicate->value.literal.string));

			if (triple->object->type == RAPTOR_TERM_TYPE_URI)
				o = QObject::tr((char*)(raptor_uri_as_string(triple->object->value.uri)));
			else
			if (triple->object->type == RAPTOR_TERM_TYPE_LITERAL)
				o = QObject::tr((char*)(triple->object->value.literal.string));

			if (!s.isEmpty()	 && !p.isEmpty() && !o.isEmpty())
			{
				//insert s as new family
				s = s.toLower();
				family1 = Ontology::nodeFamily(s);
				if (!family1)
				{
					family1 = new NodeFamily(s);
					lastReadFamilyNames << family1->name();
					Ontology::insertNodeFamily(s,family1);
				}
				if (!lastReadFamilyNames.contains(family1->name()))
					lastReadFamilyNames << family1->name();
				if (p == QObject::tr("a") || p == QObject::tr("isa") || p == QObject::tr("is a"))  //if isa relationship
				{
					o = o.toLower(); 
					family2 = Ontology::nodeFamily(o);
					if (!family2)  //insert o as new family
					{
						family2 = new NodeFamily(o);
						Ontology::insertNodeFamily(o,family2);
					}
					family1->setParent(family2);
					if (!lastReadFamilyNames.contains(family2->name()))
						lastReadFamilyNames << family2->name();
				}
				else
				if (p.toLower() == QObject::tr("synonyms") || p.toLower() == QObject::tr("synonym"))
				{
					QStringList syn = o.split(",");
					for (int i=0; i < syn.size(); ++i)
					{
						QString s2 = syn[i].trimmed().toLower();
						if (!Ontology::nodeFamily(s2))
						{
							family1->synonyms += s2;
							Ontology::insertNodeFamily(s2, family1);
						}
					}
				}
				else
				if (p.toLower() == QObject::tr("description"))
				{
					family1->description = o;
				}
				else
				if (p.toLower() == QObject::tr("units") || p.toLower() == QObject::tr("unit"))
				{
					QStringList lst = o.split(",");
					if (lst.size() > 1)
					{
						QString property = lst[0];
						QStringList unitnames = lst[1].split(" ");
						for (int i=0; i < unitnames.size(); ++i)
							if (!family1->measurementUnitOptions.contains(Unit(property, unitnames[i].trimmed())))
								family1->measurementUnitOptions += Unit(property, unitnames[i].trimmed());
					}
				}
				else
				if (p.toLower() == QObject::tr("conditions") || p.toLower() == QObject::tr("condition") ||
					 p.toLower() == QObject::tr("restrictions") || p.toLower() == QObject::tr("restriction"))
				{
					family1->restrictions += o.split(",");
				}
				else
				{
					bool ok;
					double d = o.toDouble(&ok);
					p = p.trimmed();
					p.replace(" ","_");
					if (ok)
						family1->numericalAttributes[p] = d;
					else
						family1->textAttributes[p] = o.trimmed();
				}
			}
		}

		static void read_connection_triple(void* user_data, raptor_statement* triple) 
		{
			QString s,p,o;
			ConnectionFamily *family1 = 0, *family2 = 0;			
			ParticipantRole * participantFamily1 = 0, * participantFamily2 = 0;

			if (triple->subject->type == RAPTOR_TERM_TYPE_URI)
				s = QObject::tr((char*)raptor_uri_as_string(triple->subject->value.uri));
			else
			if (triple->subject->type == RAPTOR_TERM_TYPE_LITERAL)
				s = QObject::tr((char*)(triple->subject->value.literal.string));

			if (triple->predicate->type == RAPTOR_TERM_TYPE_URI)
				p = QObject::tr((char*)raptor_uri_as_string(triple->predicate->value.uri));
			else
			if (triple->predicate->type == RAPTOR_TERM_TYPE_LITERAL)
				p = QObject::tr((char*)(triple->predicate->value.literal.string));

			if (triple->object->type == RAPTOR_TERM_TYPE_URI)
				o = QObject::tr((char*)raptor_uri_as_string(triple->object->value.uri));
			else
			if (triple->object->type == RAPTOR_TERM_TYPE_LITERAL)
				o = QObject::tr((char*)(triple->object->value.literal.string));

			if (!s.isEmpty()	 && !p.isEmpty() && !o.isEmpty())
			{
				//insert s as new family
				s = s.toLower();
				o = o.toLower(); 
				family1 = Ontology::connectionFamily(s);
				participantFamily2 = Ontology::participantRole(o);

				if (!family1 && !participantFamily2)
				{
					family1 = new ConnectionFamily(s);
					Ontology::insertConnectionFamily(s,family1);
				}
					
				if (family1 && !lastReadFamilyNames.contains(family1->name()))
					lastReadFamilyNames << family1->name();

				if (p == QObject::tr("a") || p == QObject::tr("isa") || p == QObject::tr("is a"))  //if isa relationship
				{
					if (participantFamily2)
					{
						participantFamily1 = Ontology::participantRole(s);
						if (!participantFamily1)
						{
							participantFamily1 = new ParticipantRole(s);
							Ontology::insertParticipantRole(s, participantFamily1);
						}
						participantFamily1->setParent(participantFamily2);
					}
					else
					if (family1)
					{
						family2 = Ontology::connectionFamily(o);
						if (!family2)  //insert o as new family
						{
							family2 = new ConnectionFamily(o);
							Ontology::insertConnectionFamily(o,family2);
						}
						family1->setParent(family2);
						if (!lastReadFamilyNames.contains(family2->name()))
							lastReadFamilyNames << family2->name();
					}
				}
				else
				if (family1)
				{
					if (p.toLower() == QObject::tr("synonyms"))
					{
						QStringList syn = o.split(",");
						for (int i=0; i < syn.size(); ++i)
						{
							QString s2 = syn[i].trimmed().toLower();
							if (!Ontology::connectionFamily(s2))
							{
								family1->synonyms += s2;
								Ontology::insertConnectionFamily(s2, family1);
							}
						}
					}
					else
					if (p.toLower() == QObject::tr("description"))
					{
						family1->description = o;
					}
					else
					if (p.toLower() == QObject::tr("units") || p.toLower() == QObject::tr("unit"))
					{
						QStringList lst = o.split(",");
						if (lst.size() > 1)
						{
							QString property = lst[0];
							QStringList unitnames = lst[1].split(" ");
							for (int i=0; i < unitnames.size(); ++i)
								if (!family1->measurementUnitOptions.contains(Unit(property, unitnames[i].trimmed())))
									family1->measurementUnitOptions += Unit(property, unitnames[i].trimmed());
						}
					}
					else
					if (p.toLower() == QObject::tr("participants") || p.toLower() == QObject::tr("participant"))
					{
						QStringList lst = o.split(",");
						if (lst.size() > 1)
						{
							QString type = lst[0].trimmed();
							QString role = lst[1].trimmed();
							family1->addParticipant(role, type);
						}
					}
					else
					if (p.toLower() == QObject::tr("conditions") || p.toLower() == QObject::tr("condition") ||
						 p.toLower() == QObject::tr("restrictions") || p.toLower() == QObject::tr("restriction"))
					{
						family1->restrictions += o.split(",");
					}
					else
					{
						bool ok;
						double d = o.toDouble(&ok);
						p = p.trimmed();
						p.replace(" ","_");
						if (ok)
							family1->numericalAttributes[p] = d;
						else
							family1->textAttributes[p] = o.trimmed();
					}
				}
			}
		}

		static void parse_rdf_file( void (*callback)(void*, raptor_statement*), const char * filename, const char * format)
		{
			raptor_world *world = NULL;
			raptor_parser* rdf_parser = NULL;
			unsigned char *uri_string;
			raptor_uri *uri, *base_uri;
			world = raptor_new_world();
			rdf_parser = raptor_new_parser(world, format);

			raptor_parser_set_statement_handler(rdf_parser, NULL, callback);

			uri_string = raptor_uri_filename_to_uri_string(filename);
			uri = raptor_new_uri(world, uri_string);
			base_uri = raptor_uri_copy(uri);
			raptor_parser_parse_file(rdf_parser, uri, base_uri);
			raptor_free_parser(rdf_parser);
			raptor_free_uri(base_uri);
			raptor_free_uri(uri);
			raptor_free_memory(uri_string);

			raptor_free_world(world);
		}

		QStringList Ontology::readNodes(const QString& rdf, const QString& format)
		{
			lastReadFamilyNames.clear();
			parse_rdf_file(&read_node_triple, rdf.toAscii().data(), format.toAscii().data());
			NodeFamily * family = 0;
			QList<NodeFamily*> families;
			for (int i=0; i < lastReadFamilyNames.size(); ++i)
				if ((family = Ontology::nodeFamily(lastReadFamilyNames[i])) &&
					!families.contains(family) &&
					family->parent() == 0)
					families += family;
			for (int i=0; i < families.size(); ++i)
			{
				QList<ItemFamily*> parents = families[i]->parents();
				for (int j=0; j < parents.size(); ++j)
				{
					NodeFamily * parent = NodeFamily::cast(parents[j]);
					if (parent)
					{
						if (families[i]->description.isEmpty())
							families[i]->description = parent->description;
						if (families[i]->measurementUnitOptions.isEmpty())
							families[i]->measurementUnitOptions = parent->measurementUnitOptions;
						QStringList keys(parent->textAttributes.keys());
						for (int j=0; j < keys.size(); ++j)
							if (!families[i]->textAttributes.contains(keys[j]))
								families[i]->textAttributes[ keys[j] ] = parent->textAttributes[ keys[j] ];
						keys = QStringList(parent->numericalAttributes.keys());
						for (int j=0; j < keys.size(); ++j)
							if (!families[i]->numericalAttributes.contains(keys[j]))
								families[i]->numericalAttributes[ keys[j] ] = parent->numericalAttributes[ keys[j] ];
					}
				}
				
				if (!families[i]->measurementUnitOptions.isEmpty())
					families[i]->measurementUnit = families[i]->measurementUnitOptions.first();
				families += NodeFamily::cast(families[i]->children());
			}
			return lastReadFamilyNames;
		}

		QStringList Ontology::readConnections(const QString& rdf, const QString& format)
		{
			lastReadFamilyNames.clear();

			if (!Ontology::participantRole("participant"))
				Ontology::insertParticipantRole("participant", new ParticipantRole("participant"));

			parse_rdf_file(&read_connection_triple, rdf.toAscii().data(), format.toAscii().data());
			ConnectionFamily * family = 0;
			QList<ConnectionFamily*> families;
			for (int i=0; i < lastReadFamilyNames.size(); ++i)
				if ((family = Ontology::connectionFamily(lastReadFamilyNames[i])) &&
					!families.contains(family) &&
					family->parent() == 0)
					families += family;
			for (int i=0; i < families.size(); ++i)
			{
				QList<ItemFamily*> parents = families[i]->parents();
				for (int j=0; j < parents.size(); ++j)
				{
					ConnectionFamily * parent = ConnectionFamily::cast(parents[j]);
					if (parent)
					{
						if (families[i]->participantRoles().isEmpty())
						{
							QStringList roles = parent->participantRoles();
							QStringList types = parent->participantTypes();
							for (int j=0; j < roles.size() && j < types.size(); ++j)
								families[i]->addParticipant(roles[j],types[j]);
						}
						if (families[i]->description.isEmpty())
							families[i]->description = parent->description;
						if (families[i]->measurementUnitOptions.isEmpty())
							families[i]->measurementUnitOptions = parent->measurementUnitOptions;
						QStringList keys(parent->textAttributes.keys());
						for (int j=0; j < keys.size(); ++j)
							if (!families[i]->textAttributes.contains(keys[j]))
								families[i]->textAttributes[ keys[j] ] = parent->textAttributes[ keys[j] ];
						keys = QStringList(parent->numericalAttributes.keys());
						for (int j=0; j < keys.size(); ++j)
							if (!families[i]->numericalAttributes.contains(keys[j]))
								families[i]->numericalAttributes[ keys[j] ] = parent->numericalAttributes[ keys[j] ];
					}
				}

				if (!families[i]->measurementUnitOptions.isEmpty())
					families[i]->measurementUnit = families[i]->measurementUnitOptions.first();
				families += ConnectionFamily::cast(families[i]->children());
			}
			return lastReadFamilyNames;
		}
#else
		QStringList Ontology::readNodes(const QString& rdfFile, const QString& format)
		{
			return QStringList();
		}

		QStringList Ontology::readConnections(const QString& rdfFile, const QString& format)
		{
			return QStringList();
		}
#endif

		void Ontology::cleanup()
		{
			QList<NodeFamily*> nodes = nodeFamilies.values(), visitedNodes;
			QList<ConnectionFamily*> connections = connectionFamilies.values(), visitedConnections;
			
			for (int i=0; i < nodes.size(); ++i)
				if (nodes[i] && !visitedNodes.contains(nodes[i]))
				{
					visitedNodes << nodes[i];
					delete nodes[i];
				}
			
			for (int i=0; i < connections.size(); ++i)
				if (connections[i] && !visitedConnections.contains(connections[i]))
				{
					visitedConnections << connections[i];
					delete connections[i];
				}

			nodeFamilies.clear();
			connectionFamilies.clear();
		}

		QHash<QString, NodeFamily*>  Ontology::nodeFamilies;
		QHash<QString, ConnectionFamily*>  Ontology::connectionFamilies;
		QHash<QString, ParticipantRole*>  Ontology::participantRoleFamilies;
}


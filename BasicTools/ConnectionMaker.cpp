/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

Whenever a new connection item is created, this class adds control points to the item
so that it looks appealing

****************************************************************************/
#include "GraphicsScene.h"
#include "UndoCommands.h"
#include "MainWindow.h"
#include "NodeGraphicsItem.h"
#include "ConnectionGraphicsItem.h"
#include "TextGraphicsItem.h"
#include "ConnectionMaker.h"
#include "DnaGraphicsItem.h"
#include "ConnectionsTree.h"
#include "NodesTree.h"
#include "GlobalSettings.h"
#include "Ontology.h"

namespace Tinkercell
{
	ConnectionMaker::ConnectionMaker() : Tool(tr("Connection Maker"),tr("Basic GUI"))
	{ 
		mainWindow = 0;
		//collisionDetection = 0;
	}

	bool ConnectionMaker::setMainWindow(MainWindow * main)
	{
		Tool::setMainWindow(main);
		if (mainWindow != 0)
		{
			/*
			if (main->tool("Collision Detection"))
			{
				QWidget * collisionDetection2 = main->tool("Collision Detection");
				collisionDetection = static_cast<CollisionDetection*>(collisionDetection2);
				if (collisionDetection != 0)
				{
					connect(collisionDetection,SIGNAL(nodeCollided(const QList<QGraphicsItem*>&, NodeGraphicsItem*, const QList<QPointF>&, Qt::KeyboardModifiers)),
					        			  this,SLOT(nodeCollided(const QList<QGraphicsItem*>&, NodeGraphicsItem*, const QList<QPointF>&, Qt::KeyboardModifiers)));

					connect(collisionDetection,SIGNAL(connectionCollided(const QList<QGraphicsItem*>&, ConnectionGraphicsItem *, const QList<QPointF>&, Qt::KeyboardModifiers)),
					        			  this,SLOT(connectionCollided(const QList<QGraphicsItem*>&, ConnectionGraphicsItem *, const QList<QPointF>&, Qt::KeyboardModifiers)));
				}
			}*/
			connect(main,SIGNAL(itemsInserted(GraphicsScene *, const QList<QGraphicsItem*>&, const QList<ItemHandle*>&,GraphicsScene::InsertType)),
				this,SLOT(itemsInserted(GraphicsScene *, const QList<QGraphicsItem*>&, const QList<ItemHandle*>&,GraphicsScene::InsertType)));

			return true;
		}

		return false;
	}

	void ConnectionMaker::itemsInserted(GraphicsScene * scene, const QList<QGraphicsItem*>& inserts, const QList<ItemHandle*>&, GraphicsScene::InsertType type)
	{
		if (!scene || type != GraphicsScene::NEW) return;
		static bool callBySelf = false;

		QList<QGraphicsItem*> oldItems, newItems;
		QList<QGraphicsItem*> items;

		if (!callBySelf)
		{
			for (int i=0; i < inserts.size(); ++i)
			{
				ConnectionGraphicsItem * connection = ConnectionGraphicsItem::cast(inserts.at(i));
				if (connection)
				{
					items << connection;
					/*
					ItemHandle * handle = getHandle(connection);
					if (handle && handle->family())
					{
						if (handle->family()->isA("PoPS") || (connection->className == DnaGraphicsItem::CLASSNAME))
						{
							ConnectionGraphicsItem* newConnection = new DnaGraphicsItem;
							(*newConnection) = (*connection);
							setHandle(newConnection,handle);
							newItems += newConnection;
							oldItems += connection;
							items << newConnection;
						}
					}
					*/
				}
			}
		}

		callBySelf = false;

		for (int i=0; i < items.size(); ++i)
		{
			ConnectionGraphicsItem * connection = ConnectionGraphicsItem::cast(items.at(i));
			QList<ArrowHeadItem*> arrowHeadsStart, arrowHeadsEnd;

			if (connection)
			{
				int inputs = 0, outputs = 0;

				ItemHandle * connectionHandle = connection->handle();
				QList<NodeGraphicsItem*> nodes = connection->nodes();

				if (connectionHandle)
				{
					TextDataTable & participantsTable = connectionHandle->textDataTable(ConnectionHandle::ParticipantsTableName);
					QStringList nodenames;
					for (int j=0; j < nodes.size(); ++j)
					{
						if (nodes[j] && nodes[j]->handle())
							nodenames += nodes[j]->handle()->fullName();
						else
							nodenames += tr("");

						arrowHeadsStart += 0;
						arrowHeadsEnd += 0;
					}

					for (int j=0; j < participantsTable.rows(); ++j)
					{
						QString role = participantsTable.rowName(j);
						QString nodename = participantsTable(j,0);
						int k = nodenames.indexOf(nodename);
						if (k > -1)
						{
							QPair<ArrowHeadItem*,ArrowHeadItem*> pair = getArrowHeads(role);
							arrowHeadsStart[k] = pair.first;
							arrowHeadsEnd[k] = pair.second;
						}
					}
				}

				for (int j=0; j < arrowHeadsStart.size() && j < arrowHeadsEnd.size(); ++j)
					if (arrowHeadsStart[j] == 0 && (arrowHeadsEnd.size() > 1 || arrowHeadsEnd[j] == 0))
						++inputs;
					else
						++outputs;
						
				if (inputs == 0 || outputs == 0)
					inputs = outputs = (int)(connection->curveSegments.size()/2);

				if (inputs == 1 && outputs == 1 && !arrowHeadsEnd[0] && arrowHeadsStart[1])
				{
					arrowHeadsEnd[0] = arrowHeadsStart[1];
					arrowHeadsStart[1] = 0;
				}
				else
				if (arrowHeadsEnd.size() > 2)
				{
					int k1 = 0, k2 = 0;
					k2 = -1;
					for (int j=0; j < arrowHeadsEnd.size(); ++j)
						if (arrowHeadsEnd[j])
						{
							k2 = j;
							break;						
						}
					k1 = -1;
					if (k2 > -1)
					{
						for (int j=0; j < arrowHeadsStart.size(); ++j)
						{
							if (!arrowHeadsStart[j] && !arrowHeadsEnd[j] && j != k2)
							{
								k1 = j;
								break;
							}
						}
						for (int j=0; j < arrowHeadsStart.size(); ++j)
						{
							if (arrowHeadsStart[j])
							{
								k1 = -1;
								break;
							}
						}
					}
					if (k1 > -1 && k2 > -1)
					{
						arrowHeadsStart[k1] = arrowHeadsEnd[k2];
						arrowHeadsEnd[k2] = 0;
					}
				}

				//if (!connection->isValid())
				{
					for (int j=0; j < connection->curveSegments.size(); ++j)
					{
						connection->curveSegments[j].arrowStart = 0;
						connection->curveSegments[j].arrowEnd = 0;
					}

					connection->clear();
					makeSegments(scene, connection, nodes, inputs, arrowHeadsEnd);
					connection->refresh();

					if (connection->handle()) // && connection->centerPoint())  //set location of text
					{
						TextGraphicsItem * textItem = 0;
						QPointF centerPoint = connection->centerLocation();
						QPointF p[] = { 	(centerPoint + QPointF(20.0,20.0) ),
							(centerPoint + QPointF(20.0,0.0) ),
							(centerPoint + QPointF(0.0,-20.0) ),
							(centerPoint + QPointF(0.0,20.0) ),
							(centerPoint + QPointF(-20.0,-20.0) ),
							(centerPoint + QPointF(20.0,-20.0) ),
							(centerPoint + QPointF(-20.0,20.0) ),
							(centerPoint + QPointF(-20.0,0.0) ) };
						for (int j=0; j < connection->handle()->graphicsItems.size(); ++j)
						{
							if ((textItem = TextGraphicsItem::cast(connection->handle()->graphicsItems[j])) 
								//&& !textItem->relativePosition.first
								)
							{
								for (int k=0; k < 8; ++k)
								{
									textItem->setPos( p[k] );
									if (scene->items( textItem->sceneBoundingRect() ).size() < 2)
										break;
								}
								textItem->relativePosition = 
									QPair<QGraphicsItem*,QPointF>(connection,textItem->scenePos() - centerPoint);
							}
						}
					}

					for (int j=0; j < connection->curveSegments.size() && j < arrowHeadsStart.size() && j < arrowHeadsEnd.size(); ++j)
					{
						connection->curveSegments[j].arrowStart = arrowHeadsStart[j];
						connection->curveSegments[j].arrowEnd = arrowHeadsEnd[j];
						if (connection->curveSegments[j].arrowStart) connection->curveSegments[j].arrowStart->setVisible(true);
						if (connection->curveSegments[j].arrowEnd) connection->curveSegments[j].arrowEnd->setVisible(true);
					}
					connection->refresh();
					connection->setControlPointsVisible(false);	
				}

			}
		}
		if (scene)
		{
			ConnectionGraphicsItem * connection = 0;

			if (!oldItems.isEmpty())
				scene->remove(tr("remove regular connection"),oldItems);

			if (!newItems.isEmpty())
			{
				callBySelf = true;
				scene->insert(tr("add DNA connection"),newItems);
			}

			for (int i=0; i < newItems.size(); ++i)
				if ((connection = ConnectionGraphicsItem::cast(newItems[i])))
					connection->refresh();
		}
	}
/*
	void ConnectionMaker::nodeCollided(const QList<QGraphicsItem*>& itemsMoved, NodeGraphicsItem * itemCollided, const QList<QPointF>& , Qt::KeyboardModifiers )
	{
		if (itemsMoved.size() != 1 || itemsMoved[0] == 0 || itemCollided == 0) return;
		ConnectionGraphicsItem::ControlPoint * cp = qgraphicsitem_cast<ConnectionGraphicsItem::ControlPoint*>(itemsMoved[0]);
		if (cp == 0 || cp->connectionItem == 0) return;

		cp->setPos( pointOnEdge(itemCollided->sceneBoundingRect(), cp->pos()) );
	}

	void ConnectionMaker::connectionCollided(const QList<QGraphicsItem*>& itemsMoved, ConnectionGraphicsItem * itemCollided, const QList<QPointF>& , Qt::KeyboardModifiers )
	{
		if (itemsMoved.size() != 1 || itemsMoved[0] == 0 || itemCollided == 0) return;
		ConnectionGraphicsItem::ControlPoint * cp = qgraphicsitem_cast<ConnectionGraphicsItem::ControlPoint*>(itemsMoved[0]);
		if (cp == 0 || cp->connectionItem == 0 || cp->connectionItem == itemCollided) return;

		ConnectionGraphicsItem * connection = cp->connectionItem;

		for (int i=0; i < connection->curveSegments.size(); ++i)
		{
			itemCollided->curveSegments.append(connection->curveSegments[i]);
			for (int j=0; j < connection->curveSegments[i].size(); ++j)
				connection->curveSegments[i][j]->connectionItem = itemCollided;
		}

		GraphicsScene * scene = 0;
		if (mainWindow) scene = mainWindow->currentScene();

		AddControlPointCommand ctrlPtCmnd(tr("new control point"),scene,cp);
		ctrlPtCmnd.redo();

		connection->curveSegments.clear();
		RemoveGraphicsCommand removeCmd(tr("remove"),scene,connection);
		removeCmd.redo();

		//AddControlPointCommand command(tr(),itemCollided->scene(),cp);
		//command.redo();
	}
*/
	void ConnectionMaker::setupMiddleSegment(ConnectionGraphicsItem * connection, const QList<NodeGraphicsItem*>& nodes, int inputs)
	{
		if (nodes.size() < 2 || inputs < 1 || inputs >= nodes.size()) return;

		QList<NodeGraphicsItem*> fullList,
			inputList,
			outputList;

		for (int i = 0; i < nodes.size(); ++i)
		{
			if (i < inputs)
			{
				if (nodes.at(i) && !inputList.contains(nodes.at(i)))
				{
					inputList.append(nodes.at(i));
					fullList.append(nodes.at(i));
				}
			}
			else
			{
				if (nodes.at(i) && !outputList.contains(nodes.at(i)))
				{
					outputList.append(nodes.at(i));
					fullList.append(nodes.at(i));
				}
			}
		}

		QPointF midpt1(0, 0), midpt2(0, 0), midpt(0, 0);
		for (int i=0; i < fullList.size(); ++i) //getting the midpoint for inputs and outputs
			if (fullList[i] != 0)
			{
				midpt.rx() += fullList[i]->scenePos().x();
				midpt.ry() += fullList[i]->scenePos().y();
			}

			midpt.rx() /= fullList.size();
			midpt.ry() /= fullList.size();

			QVector<QPointF> closePointsIn(inputList.size()),
									closePointsOut(outputList.size());

			for (int i = 0; i < inputList.size(); ++i)
			{
				closePointsIn[i] = pointOnEdge(inputList[i]->sceneBoundingRect(), midpt, 20.0);             
			}
			for (int i = 0; i < outputList.size(); ++i)
			{
				closePointsOut[i] = pointOnEdge(outputList[i]->sceneBoundingRect(), midpt, 20.0);
			}

			//find the midpoints again
			midpt1 = QPointF(0, 0); midpt2 = QPointF(0, 0); midpt = QPointF(0, 0);

			for (int i=0; i < inputList.size(); ++i) //getting the midpoint for inputs
			{
				midpt1.rx() += closePointsIn[i].x();
				midpt1.ry() += closePointsIn[i].y();
			}
			for (int i=0; i < outputList.size(); ++i) //getting the midpoint for outputs
			{
				midpt2.rx() += closePointsOut[i].x();
				midpt2.ry() += closePointsOut[i].y();
			}

			midpt1.rx() /= inputList.size();   //midpoint between inputs points
			midpt1.ry() /= inputList.size();

			midpt2.rx() /= outputList.size();   //midpoint between output points
			midpt2.ry() /= outputList.size();

			midpt = (midpt1 + midpt2) * 0.5;

			connection->curveSegments.clear();

			connection->curveSegments.append(ConnectionGraphicsItem::CurveSegment());

			if (inputs == 1 && nodes.size() == 2)
			{
				if (inputList[0] == outputList[0]) //self loop
				{
					connection->curveSegments[0].append(new ConnectionGraphicsItem::ControlPoint(midpt1,connection));
					connection->curveSegments[0].append(new ConnectionGraphicsItem::ControlPoint((midpt + QPointF(0,50)),connection));
					connection->curveSegments[0].append(new ConnectionGraphicsItem::ControlPoint((midpt + QPointF(0,-50)),connection));
					connection->curveSegments[0].append(new ConnectionGraphicsItem::ControlPoint(midpt2,connection));
				}
				else
				{
					connection->curveSegments[0].append(new ConnectionGraphicsItem::ControlPoint(midpt1,connection));
					connection->curveSegments[0].append(new ConnectionGraphicsItem::ControlPoint((midpt + midpt1) * 0.5,connection));
					connection->curveSegments[0].append(new ConnectionGraphicsItem::ControlPoint((midpt + midpt2) * 0.5,connection));
					connection->curveSegments[0].append(new ConnectionGraphicsItem::ControlPoint(midpt2,connection));
				}
				return;
			}
			else
			{
				connection->curveSegments[0].append(new ConnectionGraphicsItem::ControlPoint((midpt + midpt1) * 0.5,connection));
				connection->curveSegments[0].append(new ConnectionGraphicsItem::ControlPoint(midpt,connection));
				connection->curveSegments[0].append(new ConnectionGraphicsItem::ControlPoint(midpt,connection));
				connection->curveSegments[0].append(new ConnectionGraphicsItem::ControlPoint((midpt + midpt2) * 0.5,connection));    
			}

			bool loops = false;

			for (int i=0; i < inputList.size(); ++i)  //are there any loops?
			{
				if (outputList.contains(inputList[i]))
				{
					loops = true;
					break;
				}
			}

			if (loops)
			{
				float dx = connection->curveSegments[0][1]->x() - connection->curveSegments[0][2]->x(),
					dy = connection->curveSegments[0][1]->y() - connection->curveSegments[0][2]->y();

				connection->curveSegments[0][0]->setPos(
					QPointF(connection->curveSegments[0][1]->x() + 0.25*dy,
					connection->curveSegments[0][1]->y() + 0.25*dx));
				connection->curveSegments[0][3]->setPos(
					QPointF(connection->curveSegments[0][1]->x() - 0.25*dy,
					connection->curveSegments[0][1]->y() - 0.25*dx));
			}
	}

	void ConnectionMaker::makeSegments(GraphicsScene* scene,ConnectionGraphicsItem * connection, const QList<NodeGraphicsItem*>& nodes, int inputs, const QList<ArrowHeadItem*> arrowHeadsEnd)
	{
		if (!scene || nodes.size() < 2 || inputs < 1 || inputs >= nodes.size()) return;

		int k = 0;

		setupMiddleSegment(connection, nodes, inputs);

		ConnectionGraphicsItem::CurveSegment middlePiece = connection->curveSegments[0];
		connection->curveSegments.clear();

		if (middlePiece.size() != 4) 
		{
			return;
		}

		QPointF center;

		if (nodes.size() == 2)  //just two points to connect
		{
			ConnectionGraphicsItem::CurveSegment vector;
			center = nodes.at(0)->scenePos();
			vector.append(new ConnectionGraphicsItem::ControlPoint(nodes.at(0)->mapFromScene(pointOnEdge(nodes.at(0)->sceneBoundingRect(),(center + middlePiece[0]->scenePos()) * 0.5)), connection, nodes.at(0) ));
			vector.append(middlePiece[1]);
			vector.append(middlePiece[2]);   
			center = nodes.at(1)->scenePos();
			vector.append(new ConnectionGraphicsItem::ControlPoint(nodes.at(1)->mapFromScene(pointOnEdge(nodes.at(1)->sceneBoundingRect(),(center + middlePiece[3]->scenePos()) * 0.5)),connection, nodes.at(1)) );
			connection->curveSegments.append(vector);
			
			delete middlePiece[0];
			delete middlePiece[3];
		}
		else  //multiple points to connect
		{
			k = 0;
			for (int i=0; i < nodes.size(); ++i)
			{
				center = nodes.at(i)->scenePos();
				if (k < inputs)
				{
					ConnectionGraphicsItem::CurveSegment vector;
					QPointF p = pointOnEdge(nodes.at(i)->sceneBoundingRect(),(center + middlePiece[0]->scenePos()) * 0.5);
					vector.append(new ConnectionGraphicsItem::ControlPoint(nodes.at(i)->mapFromScene(p), connection, nodes.at(i) ));
					vector.append(new ConnectionGraphicsItem::ControlPoint((p + middlePiece[0]->scenePos()) * 0.5,connection));
					if (arrowHeadsEnd.at(i))
						vector.append(new ConnectionGraphicsItem::ControlPoint(*middlePiece[0]));
					else
						vector.append(middlePiece[0]);
					vector.append(middlePiece[1]);
					connection->curveSegments.append(vector);
				}
				else
				{
					ConnectionGraphicsItem::CurveSegment vector;
					QPointF p = pointOnEdge(nodes.at(i)->sceneBoundingRect(),(center + middlePiece[3]->scenePos()) * 0.5);
					vector.append(new ConnectionGraphicsItem::ControlPoint(nodes.at(i)->mapFromScene(p),connection, nodes.at(i)) );
					vector.append(new ConnectionGraphicsItem::ControlPoint((p + middlePiece[3]->scenePos()) * 0.5,connection));
					vector.append(middlePiece[3]);
					vector.append(middlePiece[1]);
					connection->curveSegments.append(vector);
				}
				++k;
			}
			delete middlePiece[2];
		}

		FixMultipleConnections(connection,nodes,inputs);

		ItemHandle * h1, *h2;

		if (connection->lineType == ConnectionGraphicsItem::line
			&& nodes.size() == 2 
			&& nodes[0] 
			&& nodes[1]
			&& (h1 = getHandle(nodes[0]))
			&& (h2 = getHandle(nodes[1])) 
			&& ((h1->isA("Part")) || (h2->isA(tr("Part")))))
		{
			ConnectionGraphicsItem::ControlPoint * cp;
			qreal x1,x2,y,dy;
			dy = nodes[0]->scenePos().y() - nodes[1]->scenePos().y();

			if (h1->isA("Part") || (dy*dy < 2.0*nodes[0]->sceneBoundingRect().height()))
			{
				if (nodes[0]->sceneBoundingRect().top() < nodes[1]->sceneBoundingRect().top())
					y = nodes[0]->sceneBoundingRect().top() - 2*nodes[0]->sceneBoundingRect().height();
				else
					y = nodes[1]->sceneBoundingRect().top() - 2*nodes[1]->sceneBoundingRect().height();

				cp = new ConnectionGraphicsItem::ControlPoint(QPointF(nodes[0]->scenePos().x(),y),connection);
				AddControlPointCommand command1(tr(""),scene,cp);
				command1.redo();
			}
			else
			{
				y = nodes[0]->scenePos().y();
			}

			//x1 = nodes[1]->sceneBoundingRect().left() + 20.0;
			//x2 = nodes[1]->sceneBoundingRect().right() - 20.0;
			x1 = x2 = nodes[1]->sceneBoundingRect().center().x();

			if ( (scene->lastPoint().x() - x1)*(scene->lastPoint().x() - x1) <
				  (scene->lastPoint().x() - x2)*(scene->lastPoint().x() - x2) )
				cp = new ConnectionGraphicsItem::ControlPoint(QPointF(x1,y),connection);
			else
				cp = new ConnectionGraphicsItem::ControlPoint(QPointF(x2,y),connection);

			if (nodes[0]->sceneBoundingRect().contains(cp->pos()))
			{
				cp->setPos( cp->pos() + QPointF( 0.0, 1.5 * nodes[0]->sceneBoundingRect().height() ) );
			}
	
			AddControlPointCommand command2(tr(""),scene,cp);
			command2.redo();
			connection->setPen(connection->defaultPen);
		}
		
		bool allLines = true;
		if (connection->lineType != ConnectionGraphicsItem::line)
		{
			for (int i=0; i < nodes.size(); ++i) //line type = line if any other connection is a line
				if (nodes[i])
				{
					QList<ConnectionGraphicsItem*> otherConnections = nodes[i]->connections();
					for (int j=0; j < otherConnections.size(); ++j)
						if (otherConnections[j] && otherConnections[j]->lineType != ConnectionGraphicsItem::line)
						{
							allLines = false;
							break;
						}
					if (!allLines)
						break;
				}
			if (allLines)
				connection->lineType = ConnectionGraphicsItem::line;
		}
	}

	void ConnectionMaker::FixMultipleConnections(ConnectionGraphicsItem * connection, const QList<NodeGraphicsItem*>& nodes, int inputs)
	{
		if (nodes.size() < 2 || inputs < 1 || inputs >= nodes.size()) return;
		QPointF center;
		float invslope, size;

		for (int i = 1; i < connection->curveSegments.size(); ++i)
		{
			for (int j = 0; j < i; ++j)
			{
				ControlPoint * p1 = connection->curveSegments[i][ connection->curveSegments[i].size() - 2 ];
				ControlPoint * p2 = connection->curveSegments[j][ connection->curveSegments[j].size() - 2 ];

				if (p1 != 0 && p2 != 0)
				{
					if ( connection->nodeAt(i) != 0 && connection->nodeAt(i) == connection->nodeAt(j) )
					{
						center = connection->nodeAt(i)->sceneBoundingRect().center();						

						size = 80.0;

						invslope = (p1->scenePos().x() - center.x()) / (p1->scenePos().y() - center.y());
						if (invslope < 0) invslope = -invslope;

						if (size > size*invslope)
						{
							p1->setPos( (QPointF( p1->scenePos().x() - size, p1->scenePos().y() -  size*invslope)));
							p2->setPos( (QPointF( p2->scenePos().x() + size, p2->scenePos().y() +  size*invslope)));
						}
						else
						{
							p1->setPos( (QPointF( p1->pos().x() - size, p1->pos().y() -  size)));
							p2->setPos( (QPointF( p2->pos().x() + size, p2->pos().y() +  size)));
						}

					}
				}
			}
		}
	}

	QPair<ArrowHeadItem*, ArrowHeadItem*> ConnectionMaker::getArrowHeads(const QString& s)
	{
		QString homeDir = GlobalSettings::homeDir();
		QString appDir = QCoreApplication::applicationDirPath();

		QPair<ArrowHeadItem*, ArrowHeadItem*> pair(0,0);
		QList<ItemFamily*> families;
		families += Ontology::participantRole(s);
		
		for (int i=0; i < families.size() && !pair.first; ++i)
			if (families[i])
			{
				QString role = families[i]->name();
				if (QFile::exists(homeDir + tr("/") + ConnectionsTree::arrowImageFile(role + tr("_start"))))
				{
					pair.first = 
						new ArrowHeadItem(homeDir + tr("/") + ConnectionsTree::arrowImageFile(role + tr("_start")));
				}
				else
				if (QFile::exists(appDir + tr("/") + ConnectionsTree::arrowImageFile(role + tr("_start"))))
				{
					pair.first =
						new ArrowHeadItem(appDir + tr("/") + ConnectionsTree::arrowImageFile(role + tr("_start")));
				}
				families += families[i]->parents();
			}


		families.clear();
		families += Ontology::participantRole(s);

		for (int i=0; i < families.size() && !pair.second; ++i)
			if (families[i])
			{
				QString role = families[i]->name();
				if (QFile::exists(homeDir + ConnectionsTree::arrowImageFile(role + tr("_end"))))
				{
					pair.second = 
						new ArrowHeadItem(homeDir + ConnectionsTree::arrowImageFile(role + tr("_end")));
				}
				else
				if (QFile::exists(appDir + ConnectionsTree::arrowImageFile(role + tr("_end"))))
				{
					pair.second = 
						new ArrowHeadItem(appDir + ConnectionsTree::arrowImageFile(role + tr("_end")));
				}
				families += families[i]->parents();
			}

		return pair;
	}

}



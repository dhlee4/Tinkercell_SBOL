/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

One of the main classes in Tinkercell.

The NodeGraphicsItem is a group made up of Shapes. Each Shape is a polygon item.
Each shape has a default color. The purpose of the default color is to allow plugins
to change color temporarily and then revert back to the default.


****************************************************************************/

#ifndef TINKERCELL_NODEGRAPHICSITEM_H
#define TINKERCELL_NODEGRAPHICSITEM_H

#include <math.h>
#include <QObject>
#include <QtDebug>
#include <QString>
#include <QColor>
#include <QPoint>
#include <QPainter>
#include <QGraphicsPolygonItem>
#include <QAbstractGraphicsShapeItem>
#include <QGraphicsItemGroup>
#include <QGraphicsRectItem>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>
#include <QTransform>
#include <QUndoCommand>
#include <QTextCursor>
#include "ControlPoint.h"

namespace Tinkercell
{
	class GraphicsScene;
	class ItemHandle;
	class ConnectionGraphicsItem;
	class NodeGraphicsItem;
	TINKERCELLCOREEXPORT void setHandle(QGraphicsItem*,ItemHandle*);

	/*! \brief A simple figure made from one or more polygons. The class can be represented in an XML file
	\ingroup core*/
	class TINKERCELLCOREEXPORT NodeGraphicsItem : public QGraphicsItemGroup
	{
	public:
		/*! \brief cast a graphics item to a node graphics item using qgraphicsitem_cast
		\param QGraphicsItem* graphics item
		\return NodeGraphicsItem* can be 0 if the cast is invalid
		*/
		static NodeGraphicsItem* cast(QGraphicsItem *);
		/*! \brief cast a list of graphics item to a list of node graphics items using qgraphicsitem_cast
		\param QList<QGraphicsItem*> graphics items
		\return QList<NodeGraphicsItem*> can be empty if no cast is invalid
		*/
		static QList<NodeGraphicsItem*> cast(const QList<QGraphicsItem*>&);
		/*! \brief get the handle of this node*/
		virtual ItemHandle * handle() const;
		/*! \brief set the handle of this node*/
		virtual void setHandle(ItemHandle *);
		/*! \brief for safe static casting*/
		QString className;
		/*! \brief for safe static casting*/
		static const QString CLASSNAME;
		/*! \brief Gets the node item from one of its child items
		* \param QGraphicsItem* the target item
		* \param bool using true here will return the node item for a control point, otherwise control points are ignored
		*/
		static NodeGraphicsItem * topLevelNodeItem(QGraphicsItem* item, bool ignoreControlPoints = false);
		/*! \brief file where the graphics item is stored*/
		QString name;
		/*! Constructor: does nothing */
		NodeGraphicsItem(QGraphicsItem * parent = 0 );
		/*! Construct from file using NodeGraphicsReader */
		NodeGraphicsItem(const QString& filename, QGraphicsItem * parent=0);
		/*! Copy Constructor */
		NodeGraphicsItem(const NodeGraphicsItem& copy);
		/*! basically does the same as copy constructor */
		virtual NodeGraphicsItem& operator = (const NodeGraphicsItem& copy);
		/*! \brief make a copy of this node item*/
		virtual NodeGraphicsItem* clone() const;
		/*! \brief paint method. Call's parent's paint after setting antialiasing to true*/
		virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option=new QStyleOptionGraphicsItem() ,QWidget *widget=0);
		/*! \brief checks that this is a valid drawable*/
		bool isValid() const;
		/*! \brief number of different type of shapes available*/
		static const int numShapeTypes = 4;
		/*! \brief arc, line, or beizier */
		enum ShapeType {arc, line, bezier, rectangle};
		/*! \brief default size for this item*/
		QSizeF defaultSize;
		/*! \brief a control point with a pointer to a NodeGraphicsItem
		\ingroup core*/
		class TINKERCELLCOREEXPORT ControlPoint : public Tinkercell::ControlPoint
		{
		public:
			/*! \brief idrawables that this control point belong in*/
			NodeGraphicsItem * nodeItem;
			/*! \brief Constructor: Setup colors and z value */
			ControlPoint(NodeGraphicsItem * idrawable_ptr = 0, QGraphicsItem * parent = 0);
			/*! \brief Copy Constructor */
			ControlPoint(const ControlPoint& copy);
			/*! Copy operator */
			virtual ControlPoint& operator = (const ControlPoint& copy);
			/*! \brief make a copy of this control point*/
			virtual Tinkercell::ControlPoint* clone() const;
			/*! \brief for enabling dynamic_cast*/
			enum { Type = UserType + 2 };
			/*! \brief for enabling dynamic_cast*/
			virtual int type() const
			{
				// Enable the use of dynamic_cast with this item.
				return Type;
			}
			/*! \brief side effect when moved. always call this after moving*/
			virtual void sideEffect();
			/*! \brief paint method. */
			virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option=new QStyleOptionGraphicsItem() ,QWidget *widget=0);
			/*! \brief same as nodeItem->handle() */
			virtual ItemHandle * handle() const;
			/*! \brief set the nodeItem->setHandle(..)*/
			virtual void setHandle(ItemHandle *);
			/*! \brief destructor*/
			~ControlPoint();
		};

		/*! \brief A closed polygon path made from arcs, lines, and beziers
		\ingroup core*/
		class TINKERCELLCOREEXPORT Shape : public QGraphicsPolygonItem
		{
		public:
			/*! \brief permanent brush for this control point*/
			QBrush defaultBrush;
			/*! \brief permanent pen for this control point*/
			QPen defaultPen;
			/*! Constructor: sets angle to 0 and scale to 1*/
			Shape(NodeGraphicsItem * idrawable_ptr = 0, QGraphicsItem * parent = 0, QGraphicsScene * scene = 0);
			/*! Copy Constructor*/
			Shape(const Shape & copy);
			/*! Copy operator */
			virtual Shape& operator = (const Shape& copy);
			/*! \brief paint method. Call's parent's paint after setting antialiasing to true*/
			//void paint(QPainter *painter, const QStyleOptionGraphicsItem *option=new QStyleOptionGraphicsItem() ,QWidget *widget=0);
			/*! the NodeGraphicsItem that this shape belongs in */
			NodeGraphicsItem *nodeItem;
			/*! is this a negative (clip out) shape */
			bool negative;
			/*! \brief Generates a new polygon using the points and types vectors
			* Precondition: points.size > 1
			* Postcondition: NA
			* \param void
			* \return void*/
			void refresh();
			/*! \brief Checks if the polygon is closed*/
			bool isClosed() const;
			/*! \brief control points defining this shape */
			QVector<ControlPoint*> controlPoints;
			/*! \brief thinckness, arc angles, etc. */
			QVector<qreal> parameters;
			/*! \brief types of shapes to draw using the control points */
			QVector<ShapeType> types;
			/*! \brief the polygon constructed from controls and types vectors */
			QPolygonF polygon;
			/*! \brief the path constructed from controls and types vectors */
			QPainterPath path;
			/*! \brief gets a path that represents this shape*/
			virtual QPainterPath shape() const;
			/*! \brief start and stop coordinates for gradient fill */
			QPair<QPointF,QPointF> gradientPoints;
			/*! \brief bounding rect*/
			virtual QRectF boundingRect() const;
			/*! \brief for enabling dynamic_cast*/
			enum { Type = UserType + 3 };
			/*! \brief for enabling dynamic_cast*/
			virtual int type() const
			{
				// Enable the use of dynamic_cast with this item.
				return Type;
			}
		protected:
			/*! \brief bounding reactangle for this shape */
			QRectF boundingRectangle;
			/*! \brief reconstruct bounding rect*/
			virtual void recomputeBoundingRect();
		};
		/*! \brief add a new control point*/
		virtual void addControlPoint(ControlPoint * control);
		/*! \brief add a shape to the set of shapes*/
		virtual void addShape(Shape* shape);
		/*! \brief remove a control point*/
		virtual void removeControlPoint(ControlPoint * control);
		/*! \brief add a shape to the set of shapes*/
		virtual void removeShape(Shape* shape);
		/*! \brief change fill color of all shapes*/
		virtual void setBrush(const QBrush& newBrush);
		/*! \brief change alpha value for brush and pen of all shapes*/
		virtual void setAlpha(int value);
		/*! \brief change outline color of all shapes*/
		virtual void setPen(const QPen& newPen);
		/*! \brief change fill color of all shapes to the default brush*/
		virtual void resetBrush();
		/*! \brief change outline color of all shapes to default pen*/
		virtual void resetPen();
		/*! \brief change color, transformation, and size to defaults*/
		virtual void resetToDefaults();
		/*! \brief gets a polygon that represents this graphicsItem*/
		virtual QPolygonF polygon() const;
		/*! \brief gets a path that represents this graphicsItem*/
		virtual QPainterPath shape() const;
		/*! \brief shapes that comprise this figure */
		QVector<Shape*> shapes;
		/*! \brief control points that control the shapes in this figure */
		QVector<ControlPoint*> controlPoints;
		/*! \brief Updates the graphicsItem by re-initializing the vector of shapes
		* Precondition: shapes.size > 1
		* Postcondition: NA
		* \param void
		* \return void*/
		virtual void refresh();
		/*! \brief normalizes a node graphics item so that its center is 0,0 and width*height is 10
		* \param node item pointer to normalize
		* \return void*/
		virtual void normalize();
		/*! \brief Clear all shapes and control points
		* \param void
		* \return void*/
		virtual void clear();
		/*! \brief bounding rect*/
		virtual QRectF boundingRect() const;
		/*! \brief Destructor: deletes all shapes and control points */
		virtual ~NodeGraphicsItem();
		/*! \brief set of control points that control the bounding box of this figure */
		QVector<ControlPoint*> boundaryControlPoints;
		/*! \brief all the control points that are used in this figure */
		virtual QList<Tinkercell::ControlPoint*> allControlPoints() const;
		/*! \brief reset of control points that control the bounding box of this figure */
		virtual void adjustBoundaryControlPoints();
		/*! \brief set boundary to match control points that control the bounding box of this figure */
		virtual void adjustToBoundaryControlPoints();
		/*! \brief set the top left and bottom right corners of this node item*/
		virtual void setBoundingRect(const QPointF&, const QPointF&);
		/*! \brief show or hide the bounding box of this figure */
		virtual void setBoundingBoxVisible(bool visible = true, bool controlPoints = true);
		/*! \brief show the bounding box of this figure. same as setBoundingBoxVisible(true) */
		void showBoundingBox(bool controlPoints = true);
		/*! \brief hide the bounding box of this figure. same as setBoundingBoxVisible(false) */
		void hideBoundingBox(bool controlPoints = true);
		/*! \brief for enabling dynamic_cast*/
		enum { Type = UserType + 4 };
		/*! \brief for enabling dynamic_cast*/
		virtual int type() const
		{
			// Enable the use of dynamic_cast with this item.
			return Type;
		}
		/*! \brief for identifying which scene this item belongs in*/
		QString groupID;
		/*! \brief get all the connection items linked to this node*/
		virtual QList<ConnectionGraphicsItem*> connections() const;
		/*! \brief get all the nodes connected to all the connections*/
		virtual QList<NodeGraphicsItem*> connectedNodes() const;
		/*! \brief get all the connection items that have an arrow associated with this node*/
		virtual QList<ConnectionGraphicsItem*> connectionsWithArrows() const;
		/*! \brief get all the connection items that do NOT have an arrow associated with this node*/
		virtual QList<ConnectionGraphicsItem*> connectionsWithoutArrows() const;
		/*! \brief get all the connection items where this node is disconnected from the main connection, e.g. modifiers*/
		virtual QList<ConnectionGraphicsItem*> connectionsDisconnected() const;
		/*! \brief get all the connection items linked to this node as a list of qgraphicsitems*/
		virtual QList<QGraphicsItem*> connectionsAsGraphicsItems() const;
		/*! \brief get all the node items that are bordering this node*/
		virtual QList<NodeGraphicsItem*> nodesAdjacent() const;
		/*! \brief get all the node items that are connected to this node directly or indirectly. 
		       only nodes that are coming in are selected (with arrows)
		       Note: if the node contains more than one connections with arrows, this list
		       returns one downstream path from the possible paths*/
		virtual QList<NodeGraphicsItem*> nodesUpstream() const;
		/*! \brief get all the node items that are connected to this node directly or indirectly. 
	       only nodes that are going out are selected (without arrows)
	       Note: if the node contains more than one connections without arrows, this list
	       returns one downstream path from the possible paths*/
		virtual QList<NodeGraphicsItem*> nodesDownstream() const;
		/*! \brief nodes to the left of this node in sequence*/
		virtual QList<NodeGraphicsItem*> nodesToLeft() const;
		/*! \brief nodes to the right of this node in sequence*/
		virtual QList<NodeGraphicsItem*> nodesToRight() const;
		/*! \brief nodes above of this node in sequence*/
		virtual QList<NodeGraphicsItem*> nodesAbove() const;
		/*! \brief nodes below of this node in sequence*/
		virtual QList<NodeGraphicsItem*> nodesBelow() const;
		/*! \brief get the shape with greatest height inside this group graphics item*/
		virtual Shape *  tallestShape() const;
		/*! \brief get the shape with greatest width inside this group graphics item*/
		virtual Shape *  longestShape() const;
		/*! \brief get the shape with lowest x value nside this group graphics item*/
		virtual Shape *  leftMostShape() const;
		/*! \brief get the shape with largest x value inside this group graphics item*/
		virtual Shape *  rightMostShape() const;
		/*! \brief get the shape with lowest y value nside this group graphics item*/
		virtual Shape *  topMostShape() const;
		/*! \brief get the shape with largest y value inside this group graphics item*/
		virtual Shape *  bottomMostShape() const;
	protected:
		/*! \brief bounding rectangle for the whole group*/
		QRectF boundingRectangle;
		/*! \brief reconstruct bounding rect*/
		virtual void recomputeBoundingRect();
		/*! \brief get pen width based on bounding rect*/
		virtual qreal getPenWidthForBoundingRect();
		/*! \brief Tinkercell object that this drawable belongs in */
		ItemHandle * itemHandle;
		/*! \brief the bounding box of this figure */
		QGraphicsRectItem * boundingBoxItem;
	};

}
#endif

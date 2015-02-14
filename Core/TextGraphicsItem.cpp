/****************************************************************************

Copyright (c) 2008 Deepak Chandran
Contact: Deepak Chandran (dchandran1@gmail.com)
See COPYRIGHT.TXT

Class for drawing text on a GraphicsScene. The text can be associated with
a handle

****************************************************************************/

#include <QPainter>
#include "Tool.h"
#include "MainWindow.h"
#include "TextGraphicsItem.h"
#include "NodeGraphicsItem.h"
#include "ConnectionGraphicsItem.h"

namespace Tinkercell
{

ItemHandle * TextGraphicsItem::handle() const
{
	return itemHandle;
}

void TextGraphicsItem::setHandle(ItemHandle * handle)
{
	if (handle != 0 && !handle->graphicsItems.contains(this))
	{
		handle->graphicsItems += this;
	}

	if (itemHandle)
	{
		if (itemHandle != handle)
		{
			itemHandle->graphicsItems.removeAll(this);
			itemHandle = handle;
		}
	}
	else
	{
		itemHandle = handle;
	}
}

/*! Constructor: sets text edit interaction */
TextGraphicsItem::TextGraphicsItem(const QString& text, QGraphicsItem* parent) :
QGraphicsTextItem(text,parent), relativePosition(QPair<QGraphicsItem*,QPointF>(0,QPointF()))
{
	setTextInteractionFlags(Qt::TextEditorInteraction);
	setFlag(QGraphicsItem::ItemIsMovable);
	//setFlag(QGraphicsItem::ItemIsSelectable);
	itemHandle = 0;
	boundingRectItem = new QGraphicsRectItem(this);
	boundingRectItem->setPen(QPen(QColor(100,100,100),2));
	boundingRectItem->setBrush(Qt::NoBrush);
	boundingRectItem->setVisible(false);
}
/*! Constructor: sets text edit interaction */
TextGraphicsItem::TextGraphicsItem(QGraphicsItem* parent) :
QGraphicsTextItem(parent), relativePosition(QPair<QGraphicsItem*,QPointF>(0,QPointF()))
{
	setTextInteractionFlags(Qt::TextEditorInteraction);
	setFlag(QGraphicsItem::ItemIsMovable);
	//setFlag(QGraphicsItem::ItemIsSelectable);
	itemHandle = 0;
	boundingRectItem = new QGraphicsRectItem(this);
	boundingRectItem->setPen(QPen(QColor(100,100,100),2));
	boundingRectItem->setBrush(Qt::NoBrush);
	boundingRectItem->setVisible(false);
}
/*! Constructor: sets text edit interaction and name of handle */
TextGraphicsItem::TextGraphicsItem(ItemHandle * handle, QGraphicsItem* parent) :
QGraphicsTextItem(parent), relativePosition(QPair<QGraphicsItem*,QPointF>(0,QPointF()))
{
	if (handle) setPlainText(handle->name);
	itemHandle = 0;
	setHandle(handle);
	setTextInteractionFlags(Qt::TextEditorInteraction);
	setFlag(QGraphicsItem::ItemIsMovable);
	//setFlag(QGraphicsItem::ItemIsSelectable);
	boundingRectItem = new QGraphicsRectItem(this);
	boundingRectItem->setPen(QPen(QColor(100,100,100),2));
	boundingRectItem->setBrush(Qt::NoBrush);
	boundingRectItem->setVisible(false);
}
/*! Copy Constructor */
TextGraphicsItem::TextGraphicsItem(const TextGraphicsItem& copy) : QGraphicsTextItem(), relativePosition(copy.relativePosition)
{
	setPos(copy.scenePos());
	setTransform(copy.transform());
	setDefaultTextColor(copy.defaultTextColor());
	setVisible(copy.isVisible());
	//setDocument(copy.document());
	setFont(copy.font());
	setHtml (copy.toHtml());
	setTextWidth(copy.textWidth());
	setTextCursor(copy.textCursor());
	setTextInteractionFlags ( copy.textInteractionFlags() );
	setFlag(QGraphicsItem::ItemIsMovable);
	//setFlag(QGraphicsItem::ItemIsSelectable);
	setPlainText(copy.toPlainText());
	itemHandle = copy.itemHandle;
	groupID = copy.groupID;

	if (itemHandle)
		setHandle(itemHandle);

	boundingRectItem = new QGraphicsRectItem(this);
	boundingRectItem->setPen(QPen(QColor(100,100,100),2));
	boundingRectItem->setBrush(Qt::NoBrush);
	boundingRectItem->setVisible(false);
}
TextGraphicsItem* TextGraphicsItem::clone()
{
	return new TextGraphicsItem(*this);
}
TextGraphicsItem::~TextGraphicsItem()
{
    if (!itemHandle) return;
    setHandle(0);
}

void TextGraphicsItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	QGraphicsTextItem::paint(painter,option,widget);
}

void TextGraphicsItem::showBorder(bool showBorder)
{
	boundingRectItem->setRect(this->boundingRect());
	boundingRectItem->setVisible(showBorder);
	update();
}

QString TextGraphicsItem::text() const
{
	return toPlainText();
}

void TextGraphicsItem::setText(const QString& s)
{
	setPlainText(s);
}

TextGraphicsItem* TextGraphicsItem::cast(QGraphicsItem * item)
{
	//if (MainWindow::invalidPointers.contains( (void*)q )) return 0;
	if (!item || ToolGraphicsItem::cast(item->topLevelItem())) return 0;
	return qgraphicsitem_cast<TextGraphicsItem*>(item);
}

QList<TextGraphicsItem*> TextGraphicsItem::cast(const QList<QGraphicsItem*>& items)
{
	QList<TextGraphicsItem*> items2;
	TextGraphicsItem * text = 0;
	for (int i=0; i < items.size(); ++i)
	{
		text = TextGraphicsItem::cast(items[i]);
		if (text && !items2.contains(text))
			items2 << text;
	}
	return items2;
}

QGraphicsItem* TextGraphicsItem::closestItem() const
{
	if (!itemHandle) return 0;
	
	QList<QGraphicsItem*> & graphicsItems = itemHandle->graphicsItems;
	QPointF p1, p2;
	p1 = this->scenePos();
	
	QGraphicsItem * closest = 0;
	double dist1 = -1, dist2;
	
	for (int i=0; i < graphicsItems.size(); ++i)
	{
		if (NodeGraphicsItem::cast(graphicsItems[i]) || ConnectionGraphicsItem::cast(graphicsItems[i]))
		{
			p2 = p1 - graphicsItems[i]->scenePos();
			dist2 = p2.x()*p2.x() + p2.y()*p2.y();
			if (dist1 < 0 || dist1 > dist2)
			{
				closest = graphicsItems[i];
				dist1 = dist2;
			}
		}
	}
	
	return closest;
}

}


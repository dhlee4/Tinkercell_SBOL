/****************************************************************************

 Copyright (c) 2008 Deepak Chandran
 Contact: Deepak Chandran (dchandran1@gmail.com)
 See COPYRIGHT.TXT
 
 This tool provides functions for the C API, allowing temporary labels to be placed on top
 of items.
 
****************************************************************************/
#include "TextGraphicsItem.h"
#include "plugins/LabelingTool.h"
#include "plots/PlotTool.h"
#include "GlobalSettings.h"

namespace Tinkercell
{	

	bool LabelingTool::ENABLE_FIRE = true;
	
	LabelingTool::LabelingTool() : Tool(tr("Labeling Tool"))
	{
		LabelingTool::fToS = new LabelingTool_FToS;
		LabelingTool::fToS->setParent(this);
		bgColor = QColor(0,0,0);
		textColor = QColor(10,255,10);
		fireNode = new NodeGraphicsItem(tr(":/images/fire.xml"));
		connect(&glowTimer,SIGNAL(frameChanged(int)),this,SLOT(makeNodeGlow(int)));
	}
	
	LabelingTool::~LabelingTool()
	{
		clearLabels();
		QSettings * settings = MainWindow::getSettings();

		settings->beginGroup("LabelingTool");
		settings->setValue("enableFire", ENABLE_FIRE);

		settings->endGroup();
		
		delete fireNode;
		for (int i=0; i < itemsToDelete.size(); ++i)
			delete itemsToDelete[i];
			
		delete settings;
	}
	
	bool LabelingTool::setMainWindow(MainWindow * main)
	{
		Tool::setMainWindow(main);

		if (mainWindow)
		{
            connect(mainWindow,SIGNAL(networkClosing(NetworkHandle * , bool *)),this,SLOT(networkClosing(NetworkHandle * , bool *)));
		
			connect(mainWindow,SIGNAL(setupFunctionPointers( QLibrary * )),this,SLOT(setupFunctionPointers( QLibrary * )));
			
			connect(mainWindow,SIGNAL(escapeSignal(const QWidget*)),this,SLOT(escapeSignal(const QWidget*)));

			connect(mainWindow,SIGNAL(mouseDoubleClicked(GraphicsScene*, QPointF, QGraphicsItem*, Qt::MouseButton, Qt::KeyboardModifiers)),this,SLOT(sceneDoubleClicked(GraphicsScene*, QPointF, QGraphicsItem*, Qt::MouseButton, Qt::KeyboardModifiers)));
			
			connect(fToS,SIGNAL(displayText(ItemHandle*,const QString&)),this,SLOT(displayText(ItemHandle*,const QString&)));
			
			connect(fToS,SIGNAL(setLabelColor(QColor,QColor)),this,SLOT(setDisplayLabelColor(QColor,QColor)));
			
			connect(fToS,SIGNAL(highlightItem(ItemHandle*,QColor)),this,SLOT(highlightItem(ItemHandle*,QColor)));
			
			connect(fToS,SIGNAL(displayFire(ItemHandle*,double)),this,SLOT(displayFire(ItemHandle*,double)));

			connect(fToS,SIGNAL(setAlpha(ItemHandle*,double)),this,SLOT(setAlpha(ItemHandle*,double)));
			
			QSettings * settings = MainWindow::getSettings();
			
			settings->beginGroup("LabelingTool");
			LabelingTool::ENABLE_FIRE = settings->value("enableFire").toBool();
			settings->endGroup();
			
			delete settings;
			
			if (mainWindow->settingsMenu)
			{
				QAction * enableFire = mainWindow->settingsMenu->addAction(tr("Enable fire effect"));
				enableFire->setCheckable(true);
				connect(enableFire, SIGNAL(toggled(bool)), this, SLOT(enableFire(bool)));
				enableFire->setChecked(LabelingTool::ENABLE_FIRE);
			}
			
			Tool * tool = mainWindow->tool("Default Plot Tool");
			if (tool && LabelingTool::ENABLE_FIRE)
			{
				PlotTool * plotTool = static_cast<PlotTool*>(tool);
				connect(plotTool,SIGNAL(displayFire(ItemHandle*,double)), this, SLOT(displayFire(ItemHandle*,double)));
	   			connect(plotTool,SIGNAL(hideFire()), this, SLOT(hideFire()));
			}
		}
		return (mainWindow != 0);
	}
	
	void LabelingTool::enableFire(bool b)
	{
		LabelingTool::ENABLE_FIRE = b;
		Tool * tool = mainWindow->tool("Default Plot Tool");
		if (tool)
		{
			PlotTool * plotTool = static_cast<PlotTool*>(tool);
			disconnect(plotTool,SIGNAL(displayFire(ItemHandle*,double)), this, SLOT(displayFire(ItemHandle*,double)));
   			disconnect(plotTool,SIGNAL(hideFire()), this, SLOT(hideFire()));
			if (LabelingTool::ENABLE_FIRE)
			{
				connect(plotTool,SIGNAL(displayFire(ItemHandle*,double)), this, SLOT(displayFire(ItemHandle*,double)));
   				connect(plotTool,SIGNAL(hideFire()), this, SLOT(hideFire()));
			}
		}
	}
	
	void LabelingTool::hideFire()
	{
		clearLabels();
	}
	
	typedef void (*tc_LabelingTool_api)(
		 void (*displayText)(long item,const char*),
		void (*displayNumber)(long item,double),
		void (*setDisplayLabelColor)(const char *, const char *),
		void (*highlight)(long,const char*),
		void (*displayFire)(long,double),
		void (*setAlpha)(long,double)
	);
	
	void LabelingTool::historyChanged( int )
	{
		escapeSignal(0);
	}
	
	void LabelingTool::setupFunctionPointers( QLibrary * library )
	{
		tc_LabelingTool_api f = (tc_LabelingTool_api)library->resolve("tc_LabelingTool_api");
		if (f)
		{
			f(
				&(_displayText),
				&(_displayNumber),
				&(_setDisplayLabelColor),
				&(_highlightItem),
				&(_displayFire),
				&(_setAlpha)
			);
		}
	}
	
	void LabelingTool::sceneDoubleClicked (GraphicsScene * scene, QPointF point, QGraphicsItem *, Qt::MouseButton, Qt::KeyboardModifiers modifiers)
	{
		clearLabels();
	}

	void LabelingTool::keyPressed(GraphicsScene * scene, QKeyEvent *)
	{
		clearLabels();
	}
	
	void LabelingTool::itemsSelected(GraphicsScene *scene, const QList<QGraphicsItem*>& items, QPointF point, Qt::KeyboardModifiers modifiers)
	{
		clearLabels();
	}
	
	void LabelingTool::escapeSignal(const QWidget*)
	{
		if (!currentScene() || !currentScene()->useDefaultBehavior())
			return;
		clearLabels();
	}

	void LabelingTool::networkClosing(NetworkHandle * , bool *)
	{
		clearLabels();
	}

	void LabelingTool::clearLabels(ItemHandle * h)
	{
		if (textItems.isEmpty() && rectItems.isEmpty() && ellipseItems.isEmpty() && fireItems.isEmpty()) return;
		
		for (int i=0; i < textItems.size(); ++i)
		{
			if (textItems[i].second && (!h || textItems[i].first==h))
			{
				if (textItems[i].second->scene())
					textItems[i].second->scene()->removeItem(textItems[i].second);
				itemsToDelete << textItems[i].second;
				textItems[i].second = 0;
			}
		}
		
		for (int i=0; i < rectItems.size(); ++i)
		{
			if (rectItems[i].second && (!h || rectItems[i].first==h))
			{
				if (rectItems[i].second->scene())
					rectItems[i].second->scene()->removeItem(rectItems[i].second);
				itemsToDelete << rectItems[i].second;
				rectItems[i].second = 0;
			}
		}
		
		for (int i=0; i < ellipseItems.size(); ++i)
		{
			if (ellipseItems[i].second && (!h || ellipseItems[i].first==h))
			{
				if (ellipseItems[i].second->scene())
					ellipseItems[i].second->scene()->removeItem(ellipseItems[i].second);
				itemsToDelete << ellipseItems[i].second;
				ellipseItems[i].second = 0;
			}
		}
		
		for (int i=0; i < fireItems.size(); ++i)
		{
			if (fireItems[i].second.first && (!h || fireItems[i].first==h))
			{
				if (fireItems[i].second.first->scene())
					fireItems[i].second.first->scene()->removeItem(fireItems[i].second.first);
				itemsToDelete << fireItems[i].second.first;
				fireItems[i].second.first = 0;
			}
		}
		
		if (!h)
		{
			textItems.clear();
			rectItems.clear();
			ellipseItems.clear();
			fireItems.clear();
			glowTimer.stop();
		}
	}

	void LabelingTool::setAlpha(ItemHandle * handle, double a)
	{
		if (!handle) return;

		if (a < 0) a = -a;		
		if (a > 0 && a <= 1) a *= 255;		
		if (a > 255) a = 255;

		QList<QGraphicsItem*> selected = handle->allGraphicsItems();
		
		for (int i=0; i < selected.size(); ++i)
		{
			NodeGraphicsItem * node = NodeGraphicsItem::cast(selected[i]);
			if (node)
			{
				node->setBoundingBoxVisible(false);
				node->setAlpha(a);
			}
			else
			{
				ConnectionGraphicsItem * connection = ConnectionGraphicsItem::cast(selected[i]);
				if (connection)
				{
					QPen pen = connection->pen();
					QColor color = pen.color();
					color.setAlpha(a);
					pen.setColor(color);
					connection->setPen(pen);
					QList<ArrowHeadItem*> arrows = connection->arrowHeads();
					if (connection->centerRegionItem)
						arrows << connection->centerRegionItem;
					
					for (int j=0; j < arrows.size(); ++j)
						if (arrows[j])
						{
							node = arrows[j];
							node->setBoundingBoxVisible(false);
							node->setAlpha(a);
						}
				}
				else
				{
					ControlPoint * cp = ControlPoint::cast(selected[i]);
					if (cp)
					{
						QPen pen = cp->pen();
						QBrush brush = cp->brush();
						QColor color = pen.color();
						color.setAlpha(a);
						pen.setColor(color);
						color = brush.color();
						color.setAlpha(a);
						brush.setColor(color);
						cp->setPen(pen);
						cp->setBrush(brush);
					}
					else
					{
						TextGraphicsItem * text = TextGraphicsItem::cast(selected[i]);
						if (text)
						{
							QColor color = text->defaultTextColor();
							color.setAlpha(a);
							text->showBorder(false);
							text->setDefaultTextColor(color);
						}
					}
				}
			}
		}
	}
	
	void LabelingTool::displayFire(ItemHandle * handle, double intensity)
	{
		if (!fireNode /*|| !LabelingTool::ENABLE_FIRE*/) return;

		if (handle)
		{
			clearLabels(handle);
		}
		
		GraphicsScene * scene = currentScene();
		if (!handle || !scene) return;
		
		if (intensity < 0.0) intensity = 0.0;
		if (intensity > 1.0) intensity = 1.0;
		
		NodeGraphicsItem * node = 0;
		ConnectionGraphicsItem * connection = 0;
		QGraphicsRectItem * rectItem = 0;
		QGraphicsSimpleTextItem * textItem = 0;
		for (int i=0; i < handle->graphicsItems.size(); ++i)
		{
			if (handle->graphicsItems[i] && handle->graphicsItems[i]->scene() == scene)
			{	
				QPointF p;
				if ((node =  NodeGraphicsItem::cast(handle->graphicsItems[i])))
					p = node->sceneBoundingRect().topRight();
				else
				{
					if ((connection = ConnectionGraphicsItem::cast(handle->graphicsItems[i])))
						p = connection->centerLocation();					
				}
				if (!p.isNull())
				{
					node = fireNode->clone();
					node->setPos(p);
					scene->addItem(node);
					node->setZValue(scene->ZValue() + 10.0);
					
					fireItems << QPair<ItemHandle*, QPair<NodeGraphicsItem*,double> >(
												handle, 
												QPair<NodeGraphicsItem*,double>(node,intensity)
											);
					if (glowTimer.state() == QTimeLine::NotRunning)
					{
						glowTimer.setFrameRange(0,254);
						glowTimer.setDirection(QTimeLine::Backward);
						//glowTimer.setUpdateInterval(100);
						glowTimer.setDuration(2000);
						glowTimer.setLoopCount(0);
						glowTimer.start();
					}
				}
			}
		}
		
	}
	
	void LabelingTool::displayText(ItemHandle* handle, const QString& text)
	{
		if (handle)
			clearLabels(handle);
		
		GraphicsScene * scene = currentScene();
		if (!handle || !scene) return;
		NodeGraphicsItem * node = 0;
		ConnectionGraphicsItem * connection = 0;
		QGraphicsRectItem * rectItem = 0;
		QGraphicsSimpleTextItem * textItem = 0;
		for (int i=0; i < handle->graphicsItems.size(); ++i)
		{
			if (handle->graphicsItems[i] && handle->graphicsItems[i]->scene() == scene)
			{	
				QPointF p;
				if ((node =  NodeGraphicsItem::cast(handle->graphicsItems[i])))
					p = node->scenePos();				
				else
				{
					if ((connection = ConnectionGraphicsItem::cast(handle->graphicsItems[i])))
						p = connection->centerLocation();					
				}
				if (!p.isNull())
				{
					textItem = new QGraphicsSimpleTextItem(text);
					textItem->setBrush(QBrush(textColor));
					textItem->setPos(p);
					QFont font = textItem->font();
					font.setPointSize(24);
					textItem->setFont(font);
					
					QRectF rect = textItem->boundingRect();
					rectItem = new QGraphicsRectItem(p.rx()-1.0,p.ry()-1.0,rect.width()+3.0,rect.height()+3.0);
					rectItem->setBrush(QBrush(bgColor));
					
					scene->addItem(rectItem);
					scene->addItem(textItem);
					rectItem->setZValue(scene->ZValue() + 10.0);
					textItem->setZValue(scene->ZValue() + 20.0);
					
					rectItems << QPair<ItemHandle*,QGraphicsRectItem*>(handle,rectItem);
					textItems << QPair<ItemHandle*,QGraphicsSimpleTextItem*>(handle,textItem);
				}
			}
		}
	}
	
	void LabelingTool::highlightItem(ItemHandle* handle, QColor color)
	{
		if (handle)
			clearLabels(handle);
		GraphicsScene * scene = currentScene();
		if (!handle || !scene) return;
		NodeGraphicsItem * node = 0;
		ConnectionGraphicsItem * connection = 0;
		QGraphicsEllipseItem * ellipseItem = 0;
		bool circle = false;
		for (int i=0; i < handle->graphicsItems.size(); ++i)
		{
			if (handle->graphicsItems[i] && handle->graphicsItems[i]->scene() == scene)
			{	
				circle = false;
				QRectF rect;
				if ((node= NodeGraphicsItem::cast(handle->graphicsItems[i])))
				{
					rect = node->sceneBoundingRect();
					rect.adjust(-50.0,-50.0,50.0,50.0);
					circle = true;
				}
				else
				{
					if ((connection = ConnectionGraphicsItem::cast(handle->graphicsItems[i])))
					{
						rect = connection->sceneBoundingRect();
						circle = true;
					}
				}
				if (circle)
				{
					ellipseItem = new QGraphicsEllipseItem(rect);
					ellipseItem->setPen(QPen(QBrush(color),4,Qt::DashDotLine));
					scene->addItem(ellipseItem);
					ellipseItems << QPair<ItemHandle*,QGraphicsEllipseItem*>(handle,ellipseItem);
				}
			}
		}
	}
	
	void LabelingTool::setDisplayLabelColor(QColor textColor,QColor bgColor)
	{	
		for (int i=0; i < textItems.size(); ++i)
		{
			if (textItems[i].second)
			{
				textItems[i].second->setBrush(QBrush(textColor));
			}
		}
		
		for (int i=0; i < rectItems.size(); ++i)
		{
			if (rectItems[i].second)
			{
				rectItems[i].second->setBrush(QBrush(bgColor));
			}
		}
	}
	
	LabelingTool_FToS * LabelingTool::fToS = 0;
	
	void LabelingTool::_displayText(long o,const char* c)
	{
		fToS->displayText(o,c);
	}
	
	void LabelingTool::_displayNumber(long o,double d)
	{
		fToS->displayNumber(o,d);
	}
	
	void LabelingTool::_setDisplayLabelColor(const char * a, const char * b)
	{
		fToS->setDisplayLabelColor(a,b);
	}
	
	void LabelingTool::_highlightItem(long o, const char * c)
	{
		fToS->highlightItem(o,c);
	}
	
	void LabelingTool_FToS::displayText(long o,const char* c)
	{
		emit displayText(ConvertValue(o),ConvertValue(c));
	}
	
	void LabelingTool_FToS::displayNumber(long o,double d)
	{
		emit displayText(ConvertValue(o),QString::number(d));
	}
	
	void LabelingTool_FToS::setDisplayLabelColor(const char* c1, const char* c2)
	{
		emit setLabelColor(QColor(tr(c1)), QColor(tr(c2)));
	}
	
	void LabelingTool_FToS::highlightItem(long o, const char* c)
	{
		emit highlightItem(ConvertValue(o),QColor(tr(c)));
	}
	
	void LabelingTool::_displayFire(long o,double d)
	{
		fToS->displayFire(o,d);
	}

	void LabelingTool::_setAlpha(long o,double d)
	{
		fToS->setAlpha(o,d);
	}
	
	void LabelingTool_FToS::displayFire(long o, double d)
	{
		//bool b = LabelingTool::ENABLE_FIRE; //turn on fire temporarily
		//LabelingTool::ENABLE_FIRE = true;
		emit displayFire(ConvertValue(o),d);
		//LabelingTool::ENABLE_FIRE = b;
	}

	void LabelingTool_FToS::setAlpha(long o, double d)
	{
		emit setAlpha(ConvertValue(o),d);
	}
	
	void LabelingTool::makeNodeGlow(int alpha)
	{
		int f = abs(alpha - 127);
		for (int i=0; i < fireItems.size(); ++i)
		{
			NodeGraphicsItem * node = fireItems[i].second.first;
			double intensity = fireItems[i].second.second;
			if (node && !node->shapes.isEmpty() && node->shapes[0])
			{
				//node->setAlpha(2*f*intensity);
				QPointF p2(node->sceneBoundingRect().bottomRight());
				QPointF p1(p2.x() - 50.0, p2.y() - 20.0 - f*intensity);
				node->setBoundingRect( p1, p2 );
			}
		}
	}
}




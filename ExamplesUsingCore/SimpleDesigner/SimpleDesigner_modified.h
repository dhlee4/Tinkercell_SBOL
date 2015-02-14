/****************************************************************************

 Copyright (C) 2009 Deepak Chandran
 Contact: Deepak Chandran (dchandran1@gmail.com)
 See COPYWRITE.TXT

This is an example application that uses the TinkerCell Core library
****************************************************************************/

#include <QButtonGroup>
#include <QAction>
#include <QActionGroup>
#include <QToolBar>
#include <QToolButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QListWidget>
#include <QGraphicsSimpleTextItem>
#include <QInputDialog>
#include "NodeGraphicsItem.h"
#include "ConnectionGraphicsItem.h"
#include "NetworkHandle.h"
#include "GraphicsScene.h"
#include "MainWindow.h"
#include "Tool.h"
#include "ConsoleWindow.h"
#include "PlotTool.h"
#include <map>
#include <string>
#include <vector>
#include "sbol.h"

typedef std::string string;

using namespace Tinkercell;
/*
class DNASequence;
class SequenceAnnotation;
class DNAComponent;

class DNASequence
{
//private:
public:
    DNASequence(string new_uri)
    {
        uri = new_uri;
        nucleotides = "aaaaaaa";
    }
    DNASequence(QString new_uri)
    {
        uri = new_uri.toStdString();
        nucleotides = "aaaaaaa";
    }
    void set_uri(string new_uri)
    {
        uri = new_uri;
    }
    void set_uri(QString new_uri)
    {
        uri = new_uri.toStdString();
    }
    void set_nucleotides(string new_nucl)
    {
        nucleotides = new_nucl;
    }
    void set_nucleotides(QString new_nucl)
    {
        nucleotides = new_nucl.toStdString();
    }
    QString get_uri()
    {
        return QString::fromStdString(uri);
    }
    string s_get_uri()
    {
        return uri;
    }
    QString get_nucleotides()
    {
        return QString::fromStdString(nucleotides);
    }

private:
    string uri;
    string nucleotides;
};

class DNAComponent
{
public: // need to be private;
    DNAComponent(string did)
    {
        source_uri = "http://partregistry.org/Part:";
        displayId = did;
        uri = source_uri+displayId;
    }
    DNAComponent(QString did)
    {
        source_uri = "http://partregistry.org/Part:";
        displayId = did.toStdString();
        uri = source_uri+displayId;
    }
    void set_displayId(string new_did)
    {
        displayId = new_did;
        uri = source_uri+displayId;
    }
    void set_displayId(QString new_did)
    {
        displayId = new_did.toStdString();
        uri = source_uri+displayId;
    }

    void set_uri(string new_uri)
    {

        uri = new_uri;

    }
    void set_uri(QString new_uri)
    {
        uri = new_uri.toStdString();
    }

    string s_get_uri()
    {
        return uri;
    }
    QString get_uri()
    {
        return QString::fromStdString(uri);
    }
    QString get_displayId()
    {
        return QString::fromStdString(displayId);
    }
    QString get_name()
    {
        return QString::fromStdString(name);
    }
    QString get_description()
    {
        return QString::fromStdString(description);
    }
private:
    string source_uri;
    string uri;
    string displayId;
    string name;
    string description;
    std::vector<string> type;
    std::vector<SequenceAnnotation*> annotations;
    DNASequence* sequence;
};


class SequenceAnnotation
{
//private:
public:
    SequenceAnnotation(string new_uri)
    {
        uri = new_uri;
    }
    SequenceAnnotation(QString new_uri)
    {
        uri = new_uri.toStdString();
    }

    void set_uri(string new_uri)
    {
        uri = new_uri;
    }
    void set_uri(QString new_uri)
    {
        uri = new_uri.toStdString();
    }
    void set_bioStart(string new_start)
    {
        bioStart = new_start;
    }
    void set_bioEnd(string new_end)
    {
        bioEnd = new_end;
    }
    void set_strand(string new_strand)
    {
        strand = new_strand;
    }

    string s_get_uri()
    {
        return uri;
    }
    QString get_uri()
    {
        return QString::fromStdString(uri);
    }
    QString get_bioStart()
    {
        return QString::fromStdString(bioStart);
    }
    QString get_bioEnd()
    {
        return QString::fromStdString(bioEnd);
    }
    QString get_strand()
    {
        return QString::fromStdString(strand);
    }

private:
    string uri;
    string bioStart;
    string bioEnd;
    string strand;
    DNAComponent* subcomponent;

};
*/
class SimpleNode: public NodeGraphicsItem
{
public:

	QGraphicsSimpleTextItem * textItem;

	SimpleNode() : NodeGraphicsItem(":/images/node.xml")
	{
		textItem = new QGraphicsSimpleTextItem;
		textItem->scale(2.0,2.0);
		addToGroup(textItem);

		refresh();
		recomputeBoundingRect();

		textItem->setPos(-boundingRect().width()/4.0,-boundingRect().height()/2.0);
		textItem->setZValue(1.0);
	}

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget)
	{
		NodeGraphicsItem::paint(painter, option, widget);
		if (textItem && itemHandle)
		{
			textItem->setText(itemHandle->name);
		}
	}

	NodeGraphicsItem* clone() const
	{
		SimpleNode * node = new SimpleNode;
		node->setPos(scenePos());
		node->adjustBoundaryControlPoints();
		return node;
	}

};

class SimpleDesigner : public Tool
{
	Q_OBJECT

public:
	SimpleDesigner();

	bool setMainWindow(MainWindow*);

public slots:

    //for DnaComponent

    void uriChanged(); // for DC and SA
    void dc_displayIdChanged();
    //void dc_nameChanged();
    //void dc_descriptionChanged();
    //void dc_typeChanged();

	void nameChanged();

	void rateChanged();

	void concentrationChanged();

	void itemsSelected(GraphicsScene * scene, const QList<QGraphicsItem*>& items, QPointF point, Qt::KeyboardModifiers modifiers);

	void itemsInserted(NetworkHandle* , const QList<ItemHandle*>& handles);

	void escapeSignal(const QWidget * sender);

	void mousePressed(GraphicsScene * , QPointF , Qt::MouseButton, Qt::KeyboardModifiers);

private slots:
	void actionTriggered(QAction*);
	void parameterItemActivated ( QListWidgetItem * item );
	void ode();
	void ssa();
	void simulate(bool stochastic);

private:
    //Document* doc;
    std::map<string,std::pair<void*,int> > global_entity;
	int mode;
	QToolBar * toolBar;
	QActionGroup * actionGroup;
	QGroupBox * groupBox1;
	QGroupBox * groupBox2;
	QGroupBox * groupBox3;

	//added
	QGroupBox * dc_groupBox;
	QGroupBox * sa_groupBox;
	QGroupBox * ds_groupBox;
	//add end

	QListWidget * listWidget;
	QLineEdit * name1;
	QLineEdit * name2;
	QLineEdit * conc;
	QLineEdit * rate;

	//add Component for DNA Component
	QLineEdit * dc_uri;
	QLineEdit * dc_displayId;
	QLineEdit * dc_name;
	QLineEdit * dc_description;
	QLineEdit * dc_type;

	QLineEdit * sa_uri;
	QLineEdit * sa_bioStart;
	QLineEdit * sa_bioEnd;
	QLineEdit * sa_strand;

	QLineEdit * ds_uri;
	QLineEdit * ds_nucleotides;

	//add end

	QAction * arrowButton;
	PlotTool * plotTool;

	QList<QGraphicsItem*> selectedItems;

	void selectItem(GraphicsScene * scene, QGraphicsItem * item, bool select = true);
	void deselectItem(GraphicsScene * scene, QGraphicsItem * item);
	void addParameters(QStringList&);
	void setToolTip(ItemHandle*);
};


/****************************************************************************

 Copyright (c) 2008 Deepak Chandran
 Contact: Deepak Chandran (dchandran1@gmail.com)
 See COPYRIGHT.TXT
 
 A tree model for storing the tree of handles

****************************************************************************/

#ifndef TINKERCELL_CONTAINERTREEMODEL_H
#define TINKERCELL_CONTAINERTREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QHash>
#include <QVariant>

#include "NetworkHandle.h"
#include "ItemHandle.h"
#include "Tool.h"

#ifndef TINKERCELLEXPORT
#ifdef Q_WS_WIN
#   if defined(TC_EXPORTS) || defined(TinkerCell_EXPORTS)
#       define TINKERCELLEXPORT __declspec(dllexport)
#   else
#       define TINKERCELLEXPORT __declspec(dllimport)
#   endif
#else
#    define TINKERCELLEXPORT
#endif
#endif

namespace Tinkercell
{
	class ContainerTreeModel;
	class ContainerTreeDelegate;

	class ContainerTreeItem
	{
		friend class ContainerTreeModel;

	public:

		ContainerTreeItem(ItemHandle* handle = 0, ContainerTreeItem *parent = 0);
		~ContainerTreeItem();

		void appendChild(ContainerTreeItem *child);

		ContainerTreeItem *child(int row);
		int childCount() const;
		int columnCount() const;
		QVariant data(int column) const;
		int row() const;
		ContainerTreeItem *parent();
		ItemHandle * handle();
		QString& text();
		void sortChildren();

	private:

		QList<ContainerTreeItem*> childItems;	
		ContainerTreeItem *parentItem;
		ItemHandle * itemHandle;
		QString attributeName;
	};

	class ContainerTreeModel : public QAbstractItemModel
	{
		Q_OBJECT

	public:
		
		ContainerTreeModel(NetworkHandle * net = 0, QObject *parent = 0);
        void reload(NetworkHandle *);
		~ContainerTreeModel();

		QVariant data(const QModelIndex &index, int role) const;
		bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
		Qt::ItemFlags flags(const QModelIndex &index) const;
		QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const;
		QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex &index) const;
		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;

		ContainerTreeItem * root();
		
		virtual void sort ( int column, Qt::SortOrder order = Qt::AscendingOrder );

	private:

		QHash<ItemHandle*,ContainerTreeItem*> treeItems;
		ContainerTreeItem *rootItem;
		NetworkHandle * network;
		ContainerTreeItem* makeBranch(ItemHandle*,ContainerTreeItem*,const QString& attribute=QString());
		ContainerTreeItem* findTreeItem(ContainerTreeItem*, ItemHandle*,const QString& attribute);

		static QHash<ContainerTreeItem*,bool> markedForDeletion;
		friend class ContainerTreeDelegate;
	};
	
	class TINKERCELLEXPORT ContainerTreeDelegate : public QItemDelegate
	{
		Q_OBJECT

	public:

		ContainerTreeDelegate(QTreeView * parent = 0);
		QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
		void setEditorData(QWidget *editor, const QModelIndex &index) const;
		void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
		void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	private:
		QTreeView * treeView;
	};

}


#endif

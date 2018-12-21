#ifndef DBTABLEWIDGET_H
#define DBTABLEWIDGET_H

#include <QTableWidget>

#include "cppNGSD_global.h"
#include "DBTable.h"

//Visualization of a database table.
class CPPNGSDSHARED_EXPORT DBTableWidget
	: public QTableWidget
{
	Q_OBJECT

public:
	DBTableWidget(QWidget* parent);

	void setData(const DBTable& table);

	void setColumnTooltips(int c, const QStringList& tooltips);

protected:
	QTableWidgetItem* createItem(const QString& text, int alignment = Qt::AlignVCenter|Qt::AlignLeft);	
	void keyPressEvent(QKeyEvent* event) override;

protected slots:
	void copyToClipboard();
	void resizeTableCells(int max_col_width=-1);

};

#endif // DBTABLEWIDGET_H

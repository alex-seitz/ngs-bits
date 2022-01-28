#ifndef PANELSTACK_H
#define PANELSTACK_H

#include <QWidget>
#include <QSplitter>
#include "ui_PanelStack.h"
#include "Panel.h"

//Stack of panels
class PanelStack
	: public QWidget
{
	Q_OBJECT

public:
	//Default constructor
	PanelStack(QWidget* parent = 0);
	//inserts panel at the given positions. If index is invalid, appends the panel to the bottom.
	void insertPanel(Panel* panel, int index);
	//appends a panel to the bottom
	void appendPanel(Panel* panel);

public slots:
	void setRegion(const BedLine& region);

signals:
	void mouseCoordinate(QString);

private:
	Ui::PanelStack ui_;
	QSplitter* splitter_;
	QSet<Panel*> panels_;
};

#endif // PANELSTACK_H

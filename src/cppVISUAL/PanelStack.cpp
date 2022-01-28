#include "PanelStack.h"
#include "GUIHelper.h"
#include <QScrollArea>
#include <QDebug>

PanelStack::PanelStack(QWidget *parent)
	: QWidget(parent)
	, ui_()
	, splitter_(new QSplitter())
	, panels_()
{
	ui_.setupUi(this);

	layout()->addWidget(splitter_);
	GUIHelper::styleSplitter(splitter_);
}

void PanelStack::insertPanel(Panel* panel, int index)
{
	//wrap panel in scroll area
	QScrollArea* area = new QScrollArea();
	area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	area->setWidgetResizable(true);
	area->setWidget(panel);

	//add scroll area to splitter
	splitter_->insertWidget(index, area);
	connect(panel, SIGNAL(mouseCoordinate(QString)), this, SIGNAL(mouseCoordinate(QString)));
	//add panel to panel set
	panels_ << panel;
}

void PanelStack::appendPanel(Panel* panel)
{
	insertPanel(panel, -1);
}

void PanelStack::setRegion(const BedLine& region)
{
	foreach(Panel* panel, panels_)
	{
		panel->setRegion(region);
	}
}

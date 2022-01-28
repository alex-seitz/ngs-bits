#ifndef PANEL_H
#define PANEL_H

#include "BedFile.h"
#include <QWidget>

//Panel interface
class Panel
	: public QWidget
{
public:
	//Sets the region of the panel
	virtual void setRegion(const BedLine& region) = 0;

signals:
	//Signal emitted when genomic coodinate of the mouse changes
	void mouseCoordinate(QString);
};

#endif // PANEL_H

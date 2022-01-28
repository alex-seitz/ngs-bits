#include "GenomeVisualizationWidget.h"
#include "ui_GenomeVisualizationWidget.h"
#include "VisualizationServiceProvider.h"
#include "BedFile.h"
#include "GUIHelper.h"
#include <QToolTip>
#include <QDebug>
#include <QMessageBox>

GenomeVisualizationWidget::GenomeVisualizationWidget(QWidget* parent)
	: QWidget(parent)
	, ui_(new Ui::GenomeVisualizationWidget)
	, settings_()
	, current_reg_()
{
	ui_->setupUi(this);

	//init chromosome list (ordered correctly)
	foreach(QByteArray chr, VisualizationServiceProvider::validChromosomes())
	{
		ui_->chr_selector->addItem(chr);
	}

	//connect signals and slots
	connect(ui_->chr_selector, SIGNAL(currentTextChanged(QString)), this, SLOT(setChromosomeRegion(QString)));
	connect(ui_->search, SIGNAL(editingFinished()), this, SLOT(search()));
	connect(ui_->zoomin_btn, SIGNAL(clicked(bool)), this, SLOT(zoomIn()));
	connect(ui_->zoomout_btn, SIGNAL(clicked(bool)), this, SLOT(zoomOut()));
	connect(this, SIGNAL(regionChanged(BedLine)), this, SLOT(updateRegionWidgets(BedLine)));
	connect(this, SIGNAL(regionChanged(BedLine)), ui_->panel_stack, SLOT(setRegion(BedLine)));
	connect(ui_->panel_stack, SIGNAL(mouseCoordinate(QString)), this, SLOT(updateCoordinateLabel(QString)));

	//add gene panel
	GenePanel* gene_panel = new GenePanel();
	ui_->panel_stack->appendPanel(gene_panel);
}

void GenomeVisualizationWidget::setRegion(const Chromosome& chr, int start, int end)
{
	//extend region to minimal region size
	int size = end-start+1;
	if (size<settings_.min_window_size)
	{
		int missing = settings_.min_window_size-size;
		start -= missing/2;
		end += missing/2;
		if (missing%2!=0)
		{
			start -= 1;
			end += 1;
		}
		size = end-start+1;
	}

	//make sure the region is in chromosome boundaries
	if (start<1)
	{
		start = 1;
		end = start + size - 1;
	}
	if (end>VisualizationServiceProvider::genomeIndex().lengthOf(chr))
	{
		end = VisualizationServiceProvider::genomeIndex().lengthOf(chr);
		start = end - size + 1;
		if (start<1) start = 1; //if size is bigger than chromosome, this can happen
	}

	//check new region is different from old
	BedLine new_reg = BedLine(chr, start, end);
	if (current_reg_==new_reg) return;

	current_reg_ = new_reg;
	emit regionChanged(current_reg_);
}

void GenomeVisualizationWidget::setChromosomeRegion(QString chr)
{
	Chromosome c(chr);
	if (!c.isValid())
	{
		QMessageBox::warning(this, __FUNCTION__, "Could not convert chromosome string '" + chr + "' to valid chromosome!");
	}

	setRegion(chr, 1, VisualizationServiceProvider::genomeIndex().lengthOf(c));
}

void GenomeVisualizationWidget::search()
{
	QByteArray text = ui_->search->text().trimmed().toLatin1();

	//chromosome
	if (VisualizationServiceProvider::validChromosomes().contains(text) || (!text.startsWith("chr") && VisualizationServiceProvider::validChromosomes().contains("chr"+text)))
	{
		setChromosomeRegion(text);
		return;
	}

	//chromosomal region
	BedLine region = BedLine::fromString(text);
	if (region.isValid())
	{
		setRegion(region.chr(), region.start(), region.end());
		return;
	}

	//gene
	if (VisualizationServiceProvider::isGeneName(text))
	{
		BedFile roi;
		foreach(const QByteArray& name, VisualizationServiceProvider::geneTranscripts(text))
		{
			const Transcript& trans = VisualizationServiceProvider::transcript(name);
			roi.append(BedLine(trans.chr(), trans.start(), trans.end()));
		}
		roi.extend(settings_.transcript_padding);
		roi.merge();
		if (roi.count()>1)
		{
			QToolTip::showText(ui_->search->mapToGlobal(QPoint(0, 0)), "Gene has several transcript regions, using the first one!\nUse transcript identifiers to select a specific transcript of the gene!" + text);
		}

		setRegion(roi[0].chr(), roi[0].start(), roi[0].end());
		return;
	}

	//transcript
	if (VisualizationServiceProvider::isTranscriptName(text))
	{
		const Transcript& trans = VisualizationServiceProvider::transcript(text);
		setRegion(trans.chr(), trans.start()-settings_.transcript_padding, trans.end()+settings_.transcript_padding);
		return;
	}

	QToolTip::showText(ui_->search->mapToGlobal(QPoint(0, 0)), "Could not find locus or feature: " + text);
}

void GenomeVisualizationWidget::zoomIn()
{
	int size = current_reg_.length();
	setRegion(current_reg_.chr(), current_reg_.start()+size/4, current_reg_.end()-size/4);
}

void GenomeVisualizationWidget::zoomOut()
{
	int size = current_reg_.length();
	setRegion(current_reg_.chr(), current_reg_.start()-size/2, current_reg_.end()+size/2);
}

void GenomeVisualizationWidget::updateRegionWidgets(const BedLine& reg)
{
	ui_->chr_selector->blockSignals(true);
	ui_->chr_selector->setCurrentText(reg.chr().strNormalized(true));
	ui_->chr_selector->blockSignals(false);

	ui_->search->blockSignals(true);
	ui_->search->setText(reg.toString(true));
	ui_->search->blockSignals(false);

	ui_->label_region_size->setText(QString::number(reg.length()));
}

void GenomeVisualizationWidget::updateCoordinateLabel(QString text)
{
	ui_->label_coordinate->setText(text);
}


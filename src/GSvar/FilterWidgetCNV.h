#ifndef FILTERWIDGETCNV_H
#define FILTERWIDGETCNV_H

#include <QWidget>
#include "ui_FilterWidgetCNV.h"
#include "BedFile.h"
#include "GeneSet.h"
#include "Phenotype.h"
#include "FilterWidget.h"

//Filter manager dock widget
class FilterWidgetCNV
	: public QWidget
{
	Q_OBJECT
	
public:
	//Default constructor
	FilterWidgetCNV(QWidget* parent = 0);

	//Resets to initial state (uncheck boxes, no ROI)
	void reset(bool clear_roi);

	//Returns the used filters
	const FilterCascade& filters() const;
	//Visually marks filters that failed.
	void markFailedFilters();

	///Returns the target region BED file. Name is empty if unset.
	const TargetRegionInfo& targetRegion() const;

	//Returns the gene names filter.
	GeneSet genes() const;
	//Returns the text filter.
	QByteArray text() const;
	//Returns the single target region filter, or an empty string if unset.
	QString region() const;
	//Sets the single target region filter, or an empty string if unset.
	void setRegion(QString region);

	//Returns selected phenotype terms.
	const PhenotypeList& phenotypes() const;
	//Sets selected phenotype terms.
	void setPhenotypes(const PhenotypeList& phenotypes);

	//Returns if only CNVs with report config should be shown
	ReportConfigFilter reportConfigurationFilter() const;

signals:
	//Signal that is emitted when a filter changes (filter cascade, gene, text, region, phenotype)
	void filtersChanged();
	//Signal is emitted when the target region changes
	void targetRegionChanged();
	/// Signal that loading phenotype data from NGSD was requested (this cannot be done inside the widget, because it knows nothing about the sample)
	void phenotypeImportNGSDRequested();
	/// Signal that requests the creation of gene overlap ToolTips
	void calculateGeneTargetRegionOverlap();

protected slots:
	void roiSelectionChanged(int index);
	void geneChanged();
	void textChanged();
	void regionChanged();
	void phenotypesChanged();
	void editPhenotypes();
	void showPhenotypeContextMenu(QPoint pos);
	void importHPO();
	void importROI();
	void importRegion();
	void importGene();
	void importText();
	void updateFilterName();
	void customFilterLoaded();
	void setFilter(int index);
	void clearTargetRegion();

private:
	//Loads filters
	void loadFilters();

	//Resets the filters without blocking signals.
	void resetSignalsUnblocked(bool clear_roi);

	//Returns the filter INI file name
	QString filterFileName() const;

	Ui::FilterWidgetCNV ui_;
	TargetRegionInfo roi_;
	GeneSet last_genes_;
	PhenotypeList phenotypes_;
};

#endif // FILTERWIDGETCNV_H

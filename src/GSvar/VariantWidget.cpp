#include "VariantWidget.h"
#include "ClassificationDialog.h"
#include "NGSD.h"
#include "GUIHelper.h"
#include "DBTableWidget.h"
#include "GSvarHelper.h"
#include "GlobalServiceProvider.h"
#include <QDialog>
#include <QMessageBox>
#include <QAction>
#include <QDesktopServices>
#include <QInputDialog>

VariantWidget::VariantWidget(const Variant& variant, QWidget *parent)
	: QWidget(parent)
	, ui_()
	, init_timer_(this, true)
	, variant_(variant)
{
	ui_.setupUi(this);
	connect(ui_.similarity, SIGNAL(clicked(bool)), this, SLOT(calculateSimilarity()));
	connect(ui_.copy_btn, SIGNAL(clicked(bool)), this, SLOT(copyToClipboard()));
	connect(ui_.update_btn, SIGNAL(clicked(bool)), this, SLOT(updateGUI()));
	connect(ui_.edit_btn, SIGNAL(clicked(bool)), this, SLOT(editComment()));
	connect(ui_.class_btn, SIGNAL(clicked(bool)), this, SLOT(editClassification()));
	connect(ui_.transcripts, SIGNAL(linkActivated(QString)), this, SLOT(openGeneTab(QString)));
	connect(ui_.af_gnomad, SIGNAL(linkActivated(QString)), this, SLOT(gnomadClicked(QString)));
	connect(ui_.pubmed, SIGNAL(linkActivated(QString)), this, SLOT(pubmedClicked(QString)));

	//add sample table context menu entries
	QAction* action = new QAction(QIcon(":/Icons/NGSD_sample.png"), "Open processed sample tab", this);
	ui_.table->addAction(action);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(openProcessedSampleTabs()));

	action = new QAction(QIcon(":/Icons/Icon.png"), "Open variant list", this);
	ui_.table->addAction(action);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(openGSvarFile()));
}

void VariantWidget::updateGUI()
{
	//get variant id
	NGSD db;
	QString variant_id = db.variantId(variant_);

	//variant base info
	ui_.variant->setText(variant_.toString());

	SqlQuery query1 = db.getQuery();
	query1.exec("SELECT * FROM variant WHERE id=" + variant_id);
	query1.next();
	ui_.af_tg->setText(query1.value("1000g").toString());
	ui_.af_gnomad->setText("<a style=\"color: #000000;\" href=\"" + variant_id + "\">" + query1.value("gnomad").toString() + "</a>");

	QVariant cadd = query1.value("cadd");
	ui_.cadd->setText(cadd.isNull() ? "" : cadd.toString());
	QVariant spliceai = query1.value("spliceai");
	ui_.spliceai->setText(spliceai.isNull() ? "" : spliceai.toString());

	QPair<int, int> counts = db.variantCounts(variant_id);
	ui_.ngsd_het->setText(QString::number(counts.first));
	ui_.ngsd_hom->setText(QString::number(counts.second));
	GSvarHelper::limitLines(ui_.comments, query1.value("comment").toString());

	//transcripts
	QStringList lines;
	QList<VariantTranscript> transcripts = Variant::parseTranscriptString(query1.value("coding").toByteArray(), true);
	foreach(const VariantTranscript& trans, transcripts)
	{
		lines << "<a href=\"" + trans.gene + "\">" + trans.gene + "</a> " + trans.id + ": " + trans.type + " " + trans.hgvs_c + " " + trans.hgvs_p;
	}
	ui_.transcripts->setText(lines.join("<br>"));

	//PubMed ids
	QStringList pubmed_ids = db.pubmedIds(variant_id);
	QStringList pubmed_links;
	foreach (const QString& id, pubmed_ids)
	{
		pubmed_links << "<a href=\"https://pubmed.ncbi.nlm.nih.gov/" + id + "/" "\">" + id + "</a>";
	}

	QString open_all;
	if (pubmed_ids.size() > 2)
	{
		open_all = "<br><a href=\"openAll\"><i>(open all (" + QString::number(pubmed_ids.size()) + " ids))</i></a>";
	}
	ui_.pubmed->setText(pubmed_links.join(", ") + open_all);
	ui_.pubmed->setToolTip(pubmed_ids.join(", "));


	//classification
	ClassificationInfo class_info = db.getClassification(variant_);
	ui_.classification->setText(class_info.classification);
	GSvarHelper::limitLines(ui_.classification_comment, class_info.comments);

	//samples table
	SqlQuery query2 = db.getQuery();
	query2.exec("SELECT processed_sample_id, genotype FROM detected_variant WHERE variant_id=" + variant_id);
	bool fill_table = true;
	if (query2.size()>100)
	{
		int res = QMessageBox::question(this, "Many variants detected.", "The variant is in NGSD " + QString::number(query2.size()) + " times.\nShowing the variant table might be slow.\nDo you want to fill the variant table?", QMessageBox::Yes, QMessageBox::No|QMessageBox::Default);
		if (res!=QMessageBox::Yes) fill_table = false;
	}

	if (fill_table)
	{
		QApplication::setOverrideCursor(Qt::BusyCursor);

		//resize table
		ui_.table->setRowCount(query2.size());

		//fill samples table
		int row = 0;
		while(query2.next())
		{
			QString ps_id = query2.value(0).toString();
			QString s_id  = db.getValue("SELECT sample_id FROM processed_sample WHERE id=" + ps_id).toString();
			SampleData s_data = db.getSampleData(s_id);
			ProcessedSampleData ps_data = db.getProcessedSampleData(ps_id);
			DiagnosticStatusData diag_data = db.getDiagnosticStatus(ps_id);
			QTableWidgetItem* item = addItem(row, 0,  ps_data.name);
			DBTableWidget::styleQuality(item, ps_data.quality);
			addItem(row, 1,  s_data.name_external);
			addItem(row, 2,  s_data.gender);
			addItem(row, 3,  s_data.quality + " / " + ps_data.quality);
			addItem(row, 4,  query2.value(1).toString());
			addItem(row, 5, ps_data.processing_system);
			addItem(row, 6, ps_data.project_name);
			addItem(row, 7, s_data.disease_group);
			addItem(row, 8, s_data.disease_status);
			QStringList pho_list;
			foreach(const Phenotype& pheno, s_data.phenotypes)
			{
				pho_list << pheno.toString();
			}
			addItem(row, 9, pho_list.join("; "));
			addItem(row, 10, diag_data.dagnostic_status);
			addItem(row, 11, diag_data.user);
			addItem(row, 12, s_data.comments, true);
			addItem(row, 13, ps_data.comments, true);

			//get causal genes from report config
			GeneSet genes_causal;
			SqlQuery query3 = db.getQuery();
			query3.exec("SELECT v.chr, v.start, v.end FROM variant v, report_configuration rc, report_configuration_variant rcv WHERE v.id=rcv.variant_id AND rcv.report_configuration_id=rc.id AND rcv.type='diagnostic variant' AND rcv.causal=1 AND rc.processed_sample_id=" + ps_id);
			while(query3.next())
			{
				genes_causal << db.genesOverlapping(query3.value(0).toByteArray(), query3.value(1).toInt(), query3.value(2).toInt(), 5000);
			}
			addItem(row, 14, genes_causal.join(','));

			//get candidate genes from report config
			GeneSet genes_candidate;
			query3.exec("SELECT v.chr, v.start, v.end FROM variant v, report_configuration rc, report_configuration_variant rcv WHERE v.id=rcv.variant_id AND rcv.report_configuration_id=rc.id AND rcv.type='candidate variant' AND rc.processed_sample_id=" + ps_id);
			while(query3.next())
			{
				genes_candidate << db.genesOverlapping(query3.value(0).toByteArray(), query3.value(1).toInt(), query3.value(2).toInt(), 5000);
			}
			addItem(row, 15, genes_candidate.join(','));

			//add report config comment of variant
			QString rc_comment;
			query3.exec("SELECT CONCAT(rcv.comments, ' // ', rcv.comments2) FROM report_configuration rc, report_configuration_variant rcv WHERE rcv.report_configuration_id=rc.id AND rc.processed_sample_id=" + ps_id + " AND rcv.variant_id=" + variant_id);
			if(query3.next())
			{
				rc_comment = query3.value(0).toString().trimmed();
			}
			addItem(row, 16, rc_comment, true);

			//validation info
			QString vv_id = db.getValue("SELECT id FROM variant_validation WHERE sample_id='" + s_id + "' AND variant_id='" + variant_id + "' AND variant_type='SNV_INDEL'").toString();
			if (!vv_id.isEmpty())
			{
				addItem(row, 17, db.getValue("SELECT status FROM variant_validation WHERE id='" + vv_id + "'").toString());
			}

			++row;
		}

		//sort by processed sample name
		ui_.table->sortByColumn(0);

		//resize table cols
		GUIHelper::resizeTableCells(ui_.table, 200);

		QApplication::restoreOverrideCursor();
	}
}

void VariantWidget::delayedInitialization()
{
	updateGUI();
}


QTableWidgetItem* VariantWidget::addItem(int r, int c, QString text, bool also_as_tooltip)
{
	if (c>=ui_.table->columnCount())
	{
		THROW(ProgrammingException, "Column '" + QString::number(c) + "' not present in variant table!");
	}

	QTableWidgetItem* item = new QTableWidgetItem(text);
	ui_.table->setItem(r, c, item);
	if (also_as_tooltip)
	{
		ui_.table->item(r, c)->setToolTip(text);
	}
	return item;
}

void VariantWidget::copyToClipboard()
{
	GUIHelper::copyToClipboard(ui_.table);
}

void VariantWidget::calculateSimilarity()
{
	QApplication::setOverrideCursor(Qt::BusyCursor);

	NGSD db;

	//get sample names and variant lists
	QStringList ps_names;
	QList<QSet<QString>> ps_vars;
	for(int i=0; i<ui_.table->rowCount(); ++i)
	{
		QString ps = ui_.table->item(i, 0)->text();
		ps_names << ps;

		QString ps_id = db.processedSampleId(ps);
		ps_vars << db.getValues("SELECT variant_id FROM detected_variant WHERE processed_sample_id=" + ps_id).toSet();
	}

	//calcualte and show overlap
	QTableWidget* table = new QTableWidget(this);
	table->setMinimumSize(700, 550);
	table->setEditTriggers(QTableWidget::NoEditTriggers);
	table->setColumnCount(6);
	table->setHorizontalHeaderLabels(QStringList() << "s1" << "#variants s1" << "s2" << "#variants s2" << "variant overlap" << "variant overlap %");
	int row = 0;
	for (int i=0; i<ps_vars.count(); ++i)
	{
		for (int j=i+1; j<ps_vars.count(); ++j)
		{
			table->setRowCount(table->rowCount()+1);
			table->setItem(row, 0, new QTableWidgetItem(ps_names[i]));
			int c_s1 = ps_vars[i].count();
			table->setItem(row, 1, new QTableWidgetItem(QString::number(c_s1)));
			table->setItem(row, 2, new QTableWidgetItem(ps_names[j]));
			int c_s2 = ps_vars[j].count();
			table->setItem(row, 3, new QTableWidgetItem(QString::number(c_s2)));
			int overlap = QSet<QString>(ps_vars[i]).intersect(ps_vars[j]).count();
			table->setItem(row, 4, new QTableWidgetItem(QString::number(overlap)));
			double overlap_perc = 100.0 * overlap / (double)std::min(c_s1, c_s2);
			auto item = new QTableWidgetItem();
			item->setData(Qt::DisplayRole, overlap_perc);
			table->setItem(row, 5, item);

			++row;
		}
	}

	table->sortByColumn(5, Qt::DescendingOrder);

	QApplication::restoreOverrideCursor();

	//show results
	auto dlg = GUIHelper::createDialog(table, "Sample correlation based on rare variants from NGSD");
	dlg->exec();
}

QList<int> VariantWidget::selectedRows() const
{
	QSet<int> set;

	foreach(QTableWidgetItem* item, ui_.table->selectedItems())
	{
		set << item->row();
	}

	return set.toList();
}

void VariantWidget::openProcessedSampleTabs()
{
	QList<int> rows = selectedRows();
	foreach(int row, rows)
	{
		QString ps = ui_.table->item(row, 0)->text();
		GlobalServiceProvider::openProcessedSampleTab(ps);
	}
}

void VariantWidget::openGeneTab(QString symbol)
{
	GlobalServiceProvider::openGeneTab(symbol);
}

void VariantWidget::openGSvarFile()
{
	QList<int> rows = selectedRows();

	//check that ony
	if (rows.count()!=1)
	{
		QMessageBox::warning(this,  "Open variant list", "Please select exactly one sampe to open!");
		return;
	}

	QString ps = ui_.table->item(rows[0], 0)->text();
	GlobalServiceProvider::openGSvarViaNGSD(ps, true);
}

void VariantWidget::editComment()
{
	try
	{
		//add variant if missing
		NGSD db;
		bool ok = true;
		QByteArray text = QInputDialog::getMultiLineText(this, "Variant comment", "Text: ", db.comment(variant_), &ok).toLatin1();
		if (!ok) return;

		db.setComment(variant_, text);
		updateGUI();
	}
	catch (DatabaseException& e)
	{
		GUIHelper::showMessage("NGSD error", e.message());
	}
}

void VariantWidget::editClassification()
{
	try
	{
		//execute dialog
		ClassificationDialog dlg(this, variant_);
		if (dlg.exec()!=QDialog::Accepted) return;

		//update NGSD
		NGSD db;
		db.setClassification(variant_, VariantList(), dlg.classificationInfo());

		updateGUI();
	}
	catch (DatabaseException& e)
	{
		GUIHelper::showMessage("NGSD error", e.message());
	}
}

void VariantWidget::gnomadClicked(QString var_id)
{
	NGSD db;
	Variant v = db.variant(var_id);
	QString link = GSvarHelper::gnomADLink(v, GSvarHelper::build());
	QDesktopServices::openUrl(QUrl(link));
}

void VariantWidget::pubmedClicked(QString link)
{
	if (link.startsWith("http")) //transcript
	{
		QDesktopServices::openUrl(QUrl(link));
	}
	else //gene
	{
		//open all publications
		QStringList pubmed_ids = ui_.pubmed->toolTip().split(", ");
		foreach (QString id, pubmed_ids)
		{
			QDesktopServices::openUrl(QUrl("https://pubmed.ncbi.nlm.nih.gov/" + id + "/"));
		}
	}
}


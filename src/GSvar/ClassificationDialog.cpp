#include "ClassificationDialog.h"
#include "NGSD.h"
#include "Helper.h"

ClassificationDialog::ClassificationDialog(QWidget* parent, const Variant& variant, bool is_somatic)
	: QDialog(parent)
	, ui_()
{
	ui_.setupUi(this);
	connect(ui_.classification, SIGNAL(currentTextChanged(QString)), this, SLOT(classificationChanged()));


	NGSD db;

	QStringList status;

	if(is_somatic)
	{
		status = db.getEnum("somatic_variant_classification", "class");
	}
	else
	{
		status = db.getEnum("variant_classification", "class");

	}

	foreach(QString s, status)
	{
		ui_.classification->addItem(s);
	}

	//set variant
	ui_.variant->setText(variant.toString());

	//get classification data from NGSD
	if(is_somatic)
	{
		ClassificationInfo class_info = db.getSomaticClassification(variant);
		ui_.classification->setCurrentText(class_info.classification);
		ui_.comment->setPlainText(class_info.comments);
	}
	else
	{
		ClassificationInfo class_info = db.getClassification(variant);
		ui_.classification->setCurrentText(class_info.classification);
		ui_.comment->setPlainText(class_info.comments);
	}

}

ClassificationInfo ClassificationDialog::classificationInfo() const
{
	return ClassificationInfo { ui_.classification->currentText(), ui_.comment->toPlainText() };
}

void ClassificationDialog::classificationChanged()
{
	QString text = ui_.comment->toPlainText().trimmed();
	if (text!="") text += "\n";
	text += "[" + ui_.classification->currentText() + "] " + Helper::userName() + " " + QDate::currentDate().toString("dd.MM.yyyy");
	ui_.comment->setPlainText(text);
}

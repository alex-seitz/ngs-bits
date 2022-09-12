#include "VariantOpenDialog.h"
#include "Helper.h"
#include "NGSD.h"
#include "GSvarHelper.h"
#include <QPushButton>

VariantOpenDialog::VariantOpenDialog(QWidget* parent)
	: QDialog(parent)
	, ui_()
	, ref_genome_idx_(Settings::string("reference_genome"))
{
	ui_.setupUi(this);

	connect(ui_.style, SIGNAL(currentIndexChanged(int)), this, SLOT(updateStyleHint()));
	connect(ui_.style, SIGNAL(currentIndexChanged(int)), this, SLOT(checkValid()));
	connect(ui_.variant, SIGNAL(editingFinished()), this, SLOT(checkValid()));
	connect(ui_.ok_btn, SIGNAL(clicked(bool)), this, SLOT(checkValidBeforeAccept()));

	updateStyleHint();
}

void VariantOpenDialog::setDefaultStyle(QString style)
{
	int index = ui_.style->findText(style);
	if (index==-1)
	{
		THROW(ArgumentException, "Invalid variant style '" + style + "' given!");
	}
	ui_.style->setCurrentIndex(index);
}

Variant VariantOpenDialog::variant()
{
	Variant output;

	QString style = ui_.style->currentText();
	if (style=="GSvar")
	{
		//Examples:
		//chr1:55042704-55042704 T>C
		//chr1:55051215-55051215 ->A
		//chr1:55056268-55056285 GGGCTTCTTGTGGCACGT>-
		output = Variant::fromString(ui_.variant->text());
	}
	else if (style=="VCF")
	{
		//Examples:
		//1	55042704	.	T	C	.	.
		//1	55051215	.	G	GA	.	.
		//1	55056267	.	CGGGCTTCTTGTGGCACGT	C	.	.
		QString text = ui_.variant->text().replace("\t", " ");

		QStringList parts = text.split(QRegExp("\\s+"));
		if (parts.count()<5) THROW(ArgumentException, "Invalid VCF variant '" + text + "': less than 5 parts found!");

		//parse parts
		Chromosome chr(parts[0]);
		int start = Helper::toInt(parts[1], "VCF start position", text);
		Sequence ref = parts[3].toUtf8().toUpper();
		Sequence alt = parts[4].toUtf8().toUpper();
		int end = start + ref.length()-1;

		output = Variant(chr, start, end, ref, alt);
		output.normalize("-", true);
	}
	else if (style=="gnomAD")
	{
		//gnomAD uses VCF-style but no ID column and '-' as separator. Examples:
		//https://gnomad.broadinstitute.org/variant/1-55042704-T-C?dataset=gnomad_r3
		//https://gnomad.broadinstitute.org/variant/1-55051215-G-GA?dataset=gnomad_r3
		//https://gnomad.broadinstitute.org/variant/1-55056267-CGGGCTTCTTGTGGCACGT-C?dataset=gnomad_r3

		QString text = ui_.variant->text().trimmed();

		QStringList parts = text.split("-");
		if (parts.count()<4) THROW(ArgumentException, "Invalid gnomAD ID '" + text + "': less than 4 parts found!");

		//parse parts
		Chromosome chr(parts[0]);
		int start = Helper::toInt(parts[1], "VCF start position", text);
		Sequence ref = parts[2].toUtf8().toUpper();
		Sequence alt = parts[3].toUtf8().toUpper();
		int end = start + ref.length()-1;

		output = Variant(chr, start, end, ref, alt);
		output.normalize("-", true);
	}
	else if (style=="HGVS.c")
	{
		//Examples:
		//ENST00000302118.5:c.208-1139T>C
		//ENST00000673903.1:c.149-1063_149-1062insA
		//ENST00000302118.5:c.996+97_996+114del
		NGSD db;

		const QMap<QByteArray, QByteArrayList>& matches = NGSHelper::transcriptMatches(GSvarHelper::build());

		QString text = ui_.variant->text().trimmed();
		int sep_pos = text.indexOf(':');
		if (sep_pos==-1) THROW(ArgumentException, "Invalid HGVS.c variant '" + text + "' - the format is [transcipt name]:[variant]");

		//remove infos in brackets
		QString transcript_name = text.left(sep_pos).trimmed();
		if (transcript_name.contains('(') && transcript_name.endsWith(')')) //remove gene name, e.g. NM_000260.4(MYO7A)
		{
			int pos = transcript_name.indexOf('(');
			transcript_name = transcript_name.left(pos).trimmed();
		}
		QString hgvs_c = text.mid(sep_pos+1).trimmed();
		if (hgvs_c.contains('(') && hgvs_c.endsWith(')')) //remove protein change, e.h. c.1348G>C (p.Glu450Gln)
		{
			int pos = hgvs_c.indexOf('(');
			hgvs_c = hgvs_c.left(pos).trimmed();
		}

		//check transcript name
		int trans_id = db.transcriptId(transcript_name, false);
		if (trans_id==-1) //not found > try to match CCDS/RefSeq to Ensembl
		{
			//remove version number (if present)
			if (transcript_name.contains("."))
			{
				transcript_name = transcript_name.left(transcript_name.indexOf('.'));
			}

			foreach(const QByteArray& match, matches[transcript_name.toUtf8()])
			{
				int match_id = db.transcriptId(match, false);
				if (match_id!=-1)
				{
					trans_id = match_id;
				}
			}
		}
		if (trans_id==-1) //not found > try if it is a gene name and use 'MANE select' transcript
		{
			int gene_id = db.geneId(transcript_name.toUtf8());
			if (gene_id!=-1)
			{
				QVariant mane_select_id = db.getValue("SELECT id FROM gene_transcript WHERE gene_id=" + QString::number(gene_id) + " AND is_mane_select=1", true);
				if (mane_select_id.isValid())
				{
					trans_id = mane_select_id.toInt();
				}
			}
		}
		if (trans_id==-1) THROW(ArgumentException, "Cannot determine Ensembl transcript in NGSD for transcript identifier '" + transcript_name + "'!");

		//convert
		Transcript transcript = db.transcript(trans_id);
		output = transcript.hgvsToVariant(hgvs_c, ref_genome_idx_);
	}

	//check that the output is valid
	output.checkValid(ref_genome_idx_);

	return output;
}

void VariantOpenDialog::updateStyleHint()
{
	QString style = ui_.style->currentText();
	if (style=="GSvar")
	{
		ui_.style_hint->setToolTip("format: chr, start, end, ref, alt (tab-separated)");
	}
	else if (style=="VCF")
	{
		ui_.style_hint->setToolTip("format: chr, pos, id, ref, alt, ... (tab-separated)");
	}
	else if (style=="gnomAD")
	{
		ui_.style_hint->setToolTip("format: chr, pos, ref, alt (separated by '-')");
	}
	else if (style=="HGVS.c")
	{
		ui_.style_hint->setToolTip("format: transcript, cDNA change (separated by ':').\nNote: if a gene name is provided instead of a transcript, the MANE select transcript of the gene is used.");
	}
}

bool VariantOpenDialog::checkValid()
{
	//special handling of empty input
	QString variant_text = ui_.variant->text().trimmed();
	if (variant_text.isEmpty())
	{
		ui_.error_message->setText("");
		return false;
	}

	//check if valid
	try
	{
		variant();
		ui_.error_message->setText("");
		return true;
	}
	catch(Exception& e) //not valid
	{
		ui_.error_message->setText(e.message());
		return false;
	}
}

void VariantOpenDialog::checkValidBeforeAccept()
{
	if (checkValid()) accept();
}

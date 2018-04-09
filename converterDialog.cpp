#include "converterDialog.h"
#include "ui_converterDialog.h"

#include <QClipboard>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QRegExp>
#include <QTextBlock>
#include <QTextFrame>
#include <QTextTable>
#include <QTextList>

namespace Details
{
    QString processTextBlock(const QTextBlock& currentBlock);
    QString processTextFrame(QTextFrame* frame);

    /**
     * @brief addMarkup
     * @param str - The string to modify by adding markup
     * @param c - The character used as markup
     */
    void addMarkup(QString& str, QChar c)
    {
        str.prepend(c);
        str.append(c);
    }

    /**
     * @return The Redmine marked-up version of the cell's contents
     */
    QString processTableCell(const QTextTableCell& cell)
    {
        QString formattedCell;

        QTextFrame::iterator it;
        for (it = cell.begin(); !(it.atEnd()); ++it)
        {
            QTextFrame *childFrame = it.currentFrame();
            QTextBlock childBlock = it.currentBlock();

            if (childFrame)
            {
                formattedCell.append(
                            Details::processTextFrame(childFrame));
            }
            else if (childBlock.isValid())
            {
                formattedCell.append(
                            Details::processTextBlock(childBlock));
            }
        }

        return formattedCell;
    }

    /**
     * @return The Redmine marked-up version of the table
     */
    QString processTable(QTextFrame* frame,
                         QTextTable* table)
    {
        Q_UNUSED(frame);

        QString tableContents;
        for (int row = 0; row < table->rows(); row++)
        {
            tableContents.append("| ");
            for (int column = 0; column < table->columns(); column++)
            {
                QTextTableCell tableCell = table->cellAt(row, column);
                if (column > 0)
                    tableContents.append(" | ");
                tableContents.append(
                            processTableCell(tableCell));
            }
            tableContents.append(" |\n");
        }

        return tableContents;
    }

    /**
     * @return The Redmine marked-up version of the fragment
     */
    QString processFragment(const QTextFragment& fragment)
    {
        QString formattedFragment;

        QString text = fragment.text();
        QTextCharFormat format = fragment.charFormat();

        formattedFragment = text;

        // Escape invalid Redmine characters
        formattedFragment.replace("*", "<notextile>*</notextile>");
        formattedFragment.replace("@", "<notextile>@</notextile>");
        formattedFragment.replace("+", "<notextile>+</notextile>");
        formattedFragment.replace("_", "<notextile>_</notextile>");
        formattedFragment.replace("-", "<notextile>-</notextile>");
        formattedFragment.replace("!", "<notextile>!</notextile>");

        // Make common list conventions Redmine lists
        formattedFragment.replace(QRegExp("^\\s*·"), "* ");
        formattedFragment.replace(
                    QRegExp("^\\s*\\d+\\.\\s"),
                    "# ");

        // URLs are often shown using fixed pitch, but
        // we won't do that because Redmine has its own
        // URL formatting rules.
        if ((format.fontFixedPitch() ||
            format.fontFamily() == "Courier") &&
            !text.startsWith("http"))
        {
            Details::addMarkup(formattedFragment, '@');
        }

        if (format.fontWeight() == QFont::Bold)
            Details::addMarkup(formattedFragment, '*');

        if (format.fontItalic())
            Details::addMarkup(formattedFragment, '_');

        if (format.fontUnderline())
            Details::addMarkup(formattedFragment, '+');

        return formattedFragment;
    }

    /**
     * @return The Redmine marked-up version of the text block
     */
    QString processTextBlock(const QTextBlock& currentBlock)
    {
        QString formattedTextBlock;

        QTextBlock::iterator it;
        for (it = currentBlock.begin(); !(it.atEnd()); ++it)
        {
            QTextFragment currentFragment = it.fragment();
            if (currentFragment.isValid())
                formattedTextBlock.append(
                            processFragment(currentFragment));
        }

        return formattedTextBlock;
    }

    /**
     * @return The Redmine marked-up version of the text frame
     */
    QString processTextFrame(QTextFrame* frame)
    {
        QString convertedText;

        QTextFrame::iterator it;
        for (it = frame->begin(); !(it.atEnd()); ++it)
        {
            QTextFrame* childFrame = it.currentFrame();
            QTextBlock childBlock = it.currentBlock();

            if (childFrame)
            {
                QTextTable* childTable = qobject_cast<QTextTable*>(childFrame);

                if (childTable)
                {
                    convertedText.append(
                                processTable(childFrame, childTable));
                }
                else
                {
                    convertedText.append(
                        processTextFrame(childFrame));
                }

            }
            else if (childBlock.isValid())
            {
                convertedText.append(
                            processTextBlock(childBlock));
                convertedText.append("\n");
            }
        }

        convertedText.append("\n");

        return convertedText;
    }

}

ConverterDialog::ConverterDialog(QWidget *parent) :
    QDialog(parent,
            Qt::WindowTitleHint |
            Qt::WindowSystemMenuHint |
            Qt::WindowCloseButtonHint),
    ui(new Ui::ConverterDialog)
{
    ui->setupUi(this);

    connect(ui->convertButton, SIGNAL(clicked()),
            this, SLOT(convertRichText()));

    QAction* restoreAction = new QAction("Show Dialog", this);
    QAction* quitAction = new QAction("Exit", this);

    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    QMenu* trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addAction(quitAction);

    QSystemTrayIcon* systray = new QSystemTrayIcon(this);
    QIcon icon(":/images/logo.ico");
    systray->setIcon(icon);
    systray->setContextMenu(trayIconMenu);
    systray->show();

    connect(systray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(systemTrayActivated(QSystemTrayIcon::ActivationReason)));
}

void ConverterDialog::systemTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)
    {
        show();
        raise();
        activateWindow();
    }
}

void ConverterDialog::closeEvent(QCloseEvent* event)
{
    hide();
    event->ignore();
}

/**
 * @brief Converts the rich text in the text field into
 *        Redmine marked-up text and puts it in the
 *        system clipboard.
 */
void ConverterDialog::convertRichText()
{
    QString convertedText;

    QTextFrame* frame =
            ui->textEdit->document()->rootFrame();

    convertedText.append(Details::processTextFrame(frame));

    QApplication::clipboard()->setText(convertedText);
}

ConverterDialog::~ConverterDialog()
{
    delete ui;
}

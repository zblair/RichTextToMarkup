#ifndef CONVERTERDIALOG_H
#define CONVERTERDIALOG_H

#include <QDialog>
#include <QSystemTrayIcon>

class QSettings;

namespace Ui {
class ConverterDialog;
}

class ConverterDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ConverterDialog(QWidget *parent = 0);
    ~ConverterDialog();

private slots:
    void convertRichText();
    void systemTrayActivated(QSystemTrayIcon::ActivationReason);

protected:
    void closeEvent(QCloseEvent* e);

private:
    Ui::ConverterDialog *ui;
};

#endif // CONVERTERDIALOG_H

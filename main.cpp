
#include "converterDialog.h"
#include <QApplication>
#include <QDebug>
#include <QSharedMemory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Don't allow two instances of this application to run at the same time
    QSharedMemory sharedMemory;
    sharedMemory.setKey("RichTextToMarkupConverter");
    if (sharedMemory.attach() || !sharedMemory.create(1))
    {
        qDebug() << "RichTextToMarkupConverter.exe alread running. Only once instance can run at a time";
        return 0; // Exit already a process running
    }

    ConverterDialog richTextConverterDialog;
    richTextConverterDialog.show();

    return a.exec();
}

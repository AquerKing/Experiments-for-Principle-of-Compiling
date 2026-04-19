#include "editordialog.h"
#include "ui_editordialog.h"

EditorDialog::EditorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditorDialog)
{
    ui->setupUi(this);
}

EditorDialog::~EditorDialog()
{
    delete ui;
}

// 实现获取列表的逻辑
QStringList EditorDialog::getLines() const
{
    // 获取所有文本
    QString text = ui->plainTextEdit->toPlainText();
    // 使用 split 方法按换行符分割
    // Qt::SkipEmptyParts 会忽略掉空的行（比如最后多余的回车）
    return text.split('\n', Qt::SkipEmptyParts);
}
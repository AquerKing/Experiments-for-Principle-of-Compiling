#ifndef EDITORDIALOG_H
#define EDITORDIALOG_H

#include <QDialog>
#include <QStringList>

// 这一步会让 ui 文件生成的类生效
namespace Ui {
class EditorDialog;
}

class EditorDialog : public QDialog
{
    Q_OBJECT

public:
    // 构造函数，支持父窗口指针
    explicit EditorDialog(QWidget *parent = nullptr);
    
    // 析构函数
    ~EditorDialog();

    // 【核心功能】获取分割后的字符串列表
    // 调用者可以通过这个函数拿到用户输入的所有行
    QStringList getLines() const;

private:
    Ui::EditorDialog *ui;
};

#endif // EDITORDIALOG_H
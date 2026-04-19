#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "grammar.h"
#include "symbols.h"
#include <QMainWindow>
#include <memory>
#include <string>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

private slots:
  void on_actionModifyGrammar_triggered();

  void on_btnAnalyze_clicked();

private:
  void addTableRow(const uint64_t &step, const std::string &stack,
                   const std::string &input, const std::string &rule,
                   const std::string &action);

  void rebuildPredictiveTable();

  bool isGrammarBuilt = false;
  Ui::MainWindow *ui;
  std::unique_ptr<SymbolManager> symbolManager;
  std::unique_ptr<PredictiveAnalysisTable> predictiveTable;
  std::unique_ptr<GenerativeExpressionPreprocessor> preprocessor;
  std::vector<GenerativeExpression> generativeExpressions;
  std::vector<std::string> generativeExpressionStrings;
};
#endif // MAINWINDOW_H

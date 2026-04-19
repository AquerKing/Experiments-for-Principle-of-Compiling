#include "mainwindow.h"
#include "editordialog.h"

#include "./ui_mainwindow.h"

#include "symbols.h"
#include "utils.h"
#include <QMessageBox>
#include <cstdint>
#include <deque>
#include <functional>
#include <queue>
#include <stack>
#include <vector>

template <typename T> class MyStack : public std::stack<T, std::vector<T>> {
public:
  std::vector<T> &get_vector() { return this->c; }

  const std::vector<T> &get_vector() const { return this->c; }
};

template <typename T, typename Container = std::deque<T>>
class MyQueue : public std::queue<T, Container> {
public:
  std::vector<T> get_vector() const {
    return std::vector<T>(this->c.begin(), this->c.end());
  }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  ui->btnAnalyze->setEnabled(isGrammarBuilt);

  symbolManager = std::make_unique<SymbolManager>();
  preprocessor =
      std::make_unique<GenerativeExpressionPreprocessor>(symbolManager.get());
  predictiveTable = std::make_unique<PredictiveAnalysisTable>();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_actionModifyGrammar_triggered() {
  EditorDialog editorDialog(this);

  if (editorDialog.exec() == QDialog::Accepted) {
    QStringList lines = editorDialog.getLines();

    generativeExpressionStrings.clear();
    for (const QString &line : lines) {
      generativeExpressionStrings.push_back(line.toStdString());
    }

    rebuildPredictiveTable();
  }
}

void MainWindow::on_btnAnalyze_clicked() {
  ui->btnAnalyze->setEnabled(false);
  ui->resultTableWidget->setRowCount(0);

  MyStack<uint64_t> symbolStack;
  MyQueue<uint64_t> inputQueue;

  symbolStack.push(Terminator::EndSymbol.SymbolId); // Push end symbol to stack
  symbolStack.push(
      symbolManager->GetStartSymbolId()); // Push start symbol to stack

  try {
    if (!ui->inputInputString->text().endsWith("#")) {
      ui->inputInputString->setText(ui->inputInputString->text() + "#");
    }
    std::string inputStr = ui->inputInputString->text().toStdString();
    for (char ch : inputStr) {
      inputQueue.push(symbolManager->GetSymbolIdByValue({ch}));
    }
  } catch (const std::invalid_argument &e) {
    QMessageBox::critical(this, "输入错误", "输入字符串包含未定义的符号");
    ui->btnAnalyze->setEnabled(true);
    return;
  }

  const size_t originalInputLength = inputQueue.size();

  static std::function<std::string()> getSymbolStackString = [&, this]() {
    return SymbolUtils::SymbolSequenceToString(symbolStack.get_vector(),
                                               *symbolManager);
  };

  static std::function<std::string()> getInputQueueString = [&, this]() {
    std::string inputQueueString = SymbolUtils::SymbolSequenceToString(
        inputQueue.get_vector(),
        *symbolManager); // Prepend empty string to avoid modifying the
                         // original vector
    return originalInputLength > inputQueueString.size()
               ? std::string(originalInputLength - inputQueueString.size(),
                             ' ') +
                     inputQueueString
               : inputQueueString; // Pad with spaces to align with the original
                                   // input
  };

  addTableRow(0,
              SymbolUtils::SymbolSequenceToString(symbolStack.get_vector(),
                                                  *symbolManager),
              SymbolUtils::SymbolSequenceToString(inputQueue.get_vector(),
                                                  *symbolManager),
              "", "初始化");

  while (!inputQueue.empty()) {
    uint64_t nextInput = inputQueue.front();
    uint64_t topSymbol = symbolStack.top();

    if (symbolManager->GetSymbol(topSymbol)->GetType() ==
            SymbolType::Terminator &&
        topSymbol != Terminator::EndSymbol.SymbolId) {
      if (nextInput != topSymbol) {
        addTableRow(ui->resultTableWidget->rowCount(), getSymbolStackString(),
                    getInputQueueString(), "ERROR", "ERROR");
        QMessageBox::critical(this, "分析错误", "输入字符串无法被文法分析");
        break;
      }

      symbolStack.pop();
      inputQueue.pop();

      addTableRow(ui->resultTableWidget->rowCount(), getSymbolStackString(),
                  getInputQueueString(), "",
                  "GENTEXT(" + symbolManager->GetSymbol(topSymbol)->ToString() +
                      ")");
    } else if (symbolManager->GetSymbol(topSymbol)->GetType() ==
               SymbolType::NonTerminator) {
      GenerativeExpression *rule =
          predictiveTable->GetItem(topSymbol, nextInput);

      if (rule == nullptr) {
        addTableRow(ui->resultTableWidget->rowCount(), getSymbolStackString(),
                    getInputQueueString(), "ERROR", "ERROR");
        QMessageBox::critical(this, "分析错误", "输入字符串无法被文法分析");
        break;
      }

      symbolStack.pop();

      std::vector<uint64_t> pushedSymbols;
      pushedSymbols.reserve(rule->GetTargets().size());
      for (auto it = rule->GetTargets().rbegin();
           it != rule->GetTargets().rend(); ++it) {
        if (*it == Terminator::Epsilon.SymbolId) {
          break;
        }
        symbolStack.push(*it);
        pushedSymbols.emplace_back(*it);
      }

      addTableRow(ui->resultTableWidget->rowCount(), getSymbolStackString(),
                  getInputQueueString(), rule->ToString(),
                  "POP" + (rule->GetTargets().size() == 1 &&
                                   rule->GetTargets().front() ==
                                       Terminator::Epsilon.SymbolId
                               ? ""
                               : ", PUSH(" +
                                     SymbolUtils::SymbolSequenceToString(
                                         pushedSymbols, *symbolManager) +
                                     ")"));
    } else if (topSymbol == Terminator::EndSymbol.SymbolId) {
      if (nextInput != topSymbol) {
        addTableRow(ui->resultTableWidget->rowCount(), getSymbolStackString(),
                    getInputQueueString(), "ERROR", "ERROR");
        QMessageBox::critical(this, "分析错误", "输入字符串无法被文法分析");
        break;
      }

      addTableRow(ui->resultTableWidget->rowCount(), getSymbolStackString(),
                  getInputQueueString(), "", "FINISH");

      symbolStack.pop();
      inputQueue.pop();
    } else {
      throw std::runtime_error("Unexpected symbol on stack");
    }
  }

  ui->btnAnalyze->setEnabled(true);
}

void MainWindow::addTableRow(const uint64_t &step, const std::string &stack,
                             const std::string &input, const std::string &rule,
                             const std::string &action) {
  int row = ui->resultTableWidget->rowCount();
  ui->resultTableWidget->insertRow(row);
  ui->resultTableWidget->setItem(row, 0,
                                 new QTableWidgetItem(QString::number(step)));
  ui->resultTableWidget->setItem(
      row, 1, new QTableWidgetItem(QString::fromStdString(stack)));
  ui->resultTableWidget->setItem(
      row, 2, new QTableWidgetItem(QString::fromStdString(input)));
  ui->resultTableWidget->setItem(
      row, 3, new QTableWidgetItem(QString::fromStdString(rule)));
  ui->resultTableWidget->setItem(
      row, 4, new QTableWidgetItem(QString::fromStdString(action)));
}

void MainWindow::rebuildPredictiveTable() {
  isGrammarBuilt = false;
  ui->btnAnalyze->setEnabled(false);
  generativeExpressions.clear();
  symbolManager.reset(new SymbolManager());
  preprocessor.reset(new GenerativeExpressionPreprocessor(symbolManager.get()));
  predictiveTable.reset(new PredictiveAnalysisTable());

  for (const std::string &exprStr : generativeExpressionStrings) {
    std::vector<GenerativeExpression> exprs =
        GrammarUtils::ParseGenerativeExpressions(exprStr, *symbolManager.get());
    generativeExpressions.insert(generativeExpressions.end(), exprs.begin(),
                                 exprs.end());
  }

  preprocessor->CalculateFirstAndFollowSets(generativeExpressions);
  predictiveTable->BuildFromPreprocessor(generativeExpressions, *preprocessor);
  isGrammarBuilt = true;
  ui->btnAnalyze->setEnabled(true);
}
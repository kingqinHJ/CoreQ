#ifndef MODERNCPPWIDGET_H
#define MODERNCPPWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QStackedWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <functional>
#include <map>

// 前向声明
class CppFeatureTable;

class ModernCppWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ModernCppWidget(QWidget *parent = nullptr);
    ~ModernCppWidget();

private slots:
    void onButtonClicked(int index);
    void onFeatureClicked(const QString& featureName, const QString& category, std::function<void(QTextEdit*)> demoFunction);

private:
    void setupUI();
    void createFeatureTables();
    
    // 创建特性演示函数
    void setupCpp11Features(CppFeatureTable* table);
    void setupCpp14Features(CppFeatureTable* table);
    void setupCpp17Features(CppFeatureTable* table);
    void setupCpp20Features(CppFeatureTable* table);
    void setupCpp23Features(CppFeatureTable* table);

private:
    QVBoxLayout* mainLayout;
    QTextEdit* outputTextEdit;
    QStackedWidget* stackedWidget;
    QHBoxLayout* buttonLayout;
    QPushButton* buttons[5];
    
    // 五个不同C++版本的页面
    CppFeatureTable* cpp11Table;
    CppFeatureTable* cpp14Table;
    CppFeatureTable* cpp17Table;
    CppFeatureTable* cpp20Table;
    CppFeatureTable* cpp23Table;
};

// 特性表格类
class CppFeatureTable : public QWidget
{
    Q_OBJECT

public:
    explicit CppFeatureTable(QWidget* parent = nullptr);
    
    void addFeature(const QString& name, const QString& category, const QString& useCase, 
                   std::function<void(QTextEdit*)> demoFunction);
    
signals:
    void featureClicked(const QString& featureName, const QString& category, std::function<void(QTextEdit*)> demoFunction);
    
private slots:
    void onCellClicked(int row, int column);
    
private:
    QTableWidget* tableWidget;
    QVBoxLayout* layout;
    
    struct FeatureInfo {
        QString name;
        QString category;
        QString useCase;
        std::function<void(QTextEdit*)> demoFunction;
    };
    
    QList<FeatureInfo> features;
};

#endif // MODERNCPPWIDGET_H 
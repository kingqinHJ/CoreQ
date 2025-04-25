#ifndef JSONWIDGET_H
#define JSONWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QMap>
#include <QLabel>

struct TV
{
    QString TVName;
    QList<QString> actress;
    QList<QString> actor;
    QMap<QString,QString>  boxOffice;
};

struct Liu{
    QString name;
    QString age;
    QString gender;
    QList<TV> tvs;
};

//Json比XML简单,要有合适的结构才能方便读取，什么时候用数组，什么时候用对象。
/**
* @brief JSON数据结构设计指南
*
* 1. JSON数组（Array）适用场景：
*    - 存储同类型的元素
*    - 需要保持元素顺序的场景
*    - 需要遍历所有元素的场景
*    - 元素数量可能会动态变化的场景
*
* 示例：
* {
*     "students": [
*         {"name": "张三", "age": 18},
*         {"name": "李四", "age": 19},
*         {"name": "王五", "age": 20}
*     ],
*     "scores": [85, 90, 95],
*     "tags": ["优秀", "良好", "及格"]
* }
*
* 2. JSON对象（Object）适用场景：
*    - 存储一个实体的多个属性
*    - 需要通过键名快速访问特定值的场景
*    - 属性名有明确含义的场景
*    - 属性数量相对固定的场景
*
* 示例：
* {
*     "student": {
*         "name": "张三",
*         "age": 18,
*         "scores": {
*             "math": 90,
*             "english": 85,
*             "physics": 95
*         }
*     }
* }
*
* 3. 选择数据结构的考虑因素：
*    - 如果数据需要频繁通过唯一标识符（如ID）查找，优先使用对象
*    - 如果数据需要保持顺序或需要遍历，优先使用数组
*    - 如果数据项的结构完全相同，优先使用数组
*    - 如果数据项的结构可能不同，优先使用对象
*
* 4. 最佳实践示例：
*
* // 使用数组的场景
* {
*     "products": [
*         {"id": 1, "name": "手机", "price": 1000},
*         {"id": 2, "name": "电脑", "price": 2000}
*     ]
* }
*
* // 使用对象的场景
* {
*     "product": {
*         "id": 1,
*         "name": "手机",
*         "price": 1000,
*         "specs": {
*             "color": "黑色",
*             "size": "6.1寸"
*         }
*     }
* }
*
* // 混合使用的场景
* {
*     "store": {
*         "name": "电子商城",
*         "products": {
*             "phones": [
*                 {"id": 1, "name": "iPhone"},
*                 {"id": 2, "name": "Samsung"}
*             ],
*             "computers": [
*                 {"id": 3, "name": "MacBook"},
*                 {"id": 4, "name": "ThinkPad"}
*             ]
*         }
*     }
* }
*
* 5. 需要避免的常见错误：
*    - 不要用数组存储不同结构的数据
*    - 不要用对象存储需要保持顺序的数据
*    - 不要用数组存储需要通过键名快速访问的数据
*    - 不要用对象存储大量同类型的数据
*/

class JsonWidget : public QWidget
{
    Q_OBJECT

public:
    JsonWidget(QWidget *parent = nullptr);
    ~JsonWidget();

private slots:
    void onGenerateJsonClicked();
    void onReadJsonFileClicked();

private:
    void setupUI();

    QVBoxLayout *mainLayout;
    QPushButton *generateJsonBtn;
    QPushButton *readJsonFileBtn;
    QTextEdit *resultTextEdit;
    QLabel *titleLabel;
};

#endif // JSONWIDGET_H

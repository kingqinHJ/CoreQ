#include "jsonwidget.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QFileDialog>
#include <QDebug>
#include <QJsonArray>
#include<QJsonDocument>

JsonWidget::JsonWidget(QWidget *parent)
    : QWidget(parent)
    , mainLayout(new QVBoxLayout(this))
    , generateJsonBtn(new QPushButton("生成并写入JSON", this))
    , readJsonFileBtn(new QPushButton("读取JSON文件", this))
    , resultTextEdit(new QTextEdit(this))
    , titleLabel(new QLabel("JSON操作演示", this))
{
    setupUI();

    // 连接信号和槽
    connect(generateJsonBtn, &QPushButton::clicked, this, &JsonWidget::onGenerateJsonClicked);
    connect(readJsonFileBtn, &QPushButton::clicked, this, &JsonWidget::onReadJsonFileClicked);
}

JsonWidget::~JsonWidget()
{
}

void JsonWidget::setupUI()
{
    // 设置窗口标题
    setWindowTitle("JSON操作示例");

    // 添加标题标签
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    // 设置结果标签
    resultTextEdit->setFrameShape(QFrame::Panel);
    resultTextEdit->setFrameShadow(QFrame::Sunken);
    resultTextEdit->setAlignment(Qt::AlignCenter);
    resultTextEdit->setText("点击按钮执行JSON操作");

    // 添加到布局
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(resultTextEdit);
    mainLayout->addWidget(generateJsonBtn);
    mainLayout->addWidget(readJsonFileBtn);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void JsonWidget::onGenerateJsonClicked()
{
    // 创建JSON对象,JSON就是Object和Array嵌套
    QJsonObject jsonObj;
    jsonObj["name"] = "Miss LIU";
    jsonObj["age"] = 36;
    jsonObj["gender"] = "woman";
    QJsonObject tvObject;
    for(int i=0;i<3;++i)
    {
        QJsonObject oneTVObject;
        QJsonArray actressArray;
        actressArray.append("actress"+tr("%1").arg(0));
        oneTVObject.insert("actress",actressArray);
        QJsonArray actorArray;
        actorArray.append("actor"+tr("%1").arg(i+1));
        actorArray.append("actor"+tr("%1").arg(i+2));
        oneTVObject.insert("actor",actorArray);
        QJsonObject boxOffice;
        boxOffice.insert("china","25M");
        boxOffice.insert("US","60M");
        oneTVObject.insert("boxOffice",boxOffice);
        tvObject.insert(tr("TV%1").arg(i),oneTVObject);
    }
    jsonObj.insert("TV PLAY",tvObject);
    // 转换为JSON文档
    QJsonDocument doc(jsonObj);
    QString jsonString = doc.toJson(QJsonDocument::Indented);

    // 显示生成的JSON
    resultTextEdit->append(jsonString);
    qDebug() << "生成的JSON:" << jsonString;

    // 获取保存文件路径
    QString filePath = QFileDialog::getSaveFileName(this, "保存JSON文件",
                                                    QDir::homePath(),
                                                    "JSON文件 (*.json)");

    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(jsonString.toUtf8());
            file.close();
            resultTextEdit->append("JSON文件已保存到:\n" + filePath);
            qDebug() << "JSON文件已保存到:" << filePath;
        } else {
            resultTextEdit->append("保存文件失败: " + file.errorString());
            qDebug() << "保存文件失败:" << file.errorString();
        }
    }
}

void JsonWidget::onReadJsonFileClicked()
{
    resultTextEdit->clear();
    QString filePath = QFileDialog::getOpenFileName(this, "选择JSON文件",
                                                    QDir::homePath(),
                                                    "JSON文件 (*.json)");
    if(filePath.isEmpty())
    {
        qDebug() << "文件路径为空";
        return;
    }


    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "文件打开失败";
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc(QJsonDocument::fromJson(jsonData));
    QJsonObject objRoot = doc.object();
    if(doc.isNull())
    {
        qDebug() << "JSON文件为空";
        return;
    }

    if(!doc.isObject())
    {
        qDebug() << "JSON文件为空";
        return ;
    }

    Liu liu;
    liu.name=objRoot["name"].toString();
    liu.age=objRoot["age"].toInt();
    liu.gender=objRoot["gender"].toString();
    QJsonObject tvObject=objRoot["TV PLAY"].toObject();
    foreach(QString key,tvObject.keys())
    {
        TV tv;
        QJsonObject oneTVObject=tvObject[key].toObject();
        tv.TVName=key;
        QJsonArray actressArray=oneTVObject["actress"].toArray();
        foreach(QJsonValue val,actressArray)
        {
            tv.actress.append(val.toString());
        }
        QJsonArray actorArray=oneTVObject["actor"].toArray();
        foreach(QJsonValue val,actorArray)
        {
            tv.actor.append(val.toString());
        }
        QJsonObject boxOffice=oneTVObject["boxOffice"].toObject();
        foreach(QString key,boxOffice.keys())
        {
            tv.boxOffice.insert(key,boxOffice[key].toString());
        }
        liu.tvs.append(tv);
    }
    resultTextEdit->append(tr("姓名:%1\n年龄:%2\n性别:%3\n电视剧:%4").arg(liu.name).arg(liu.age).arg(liu.gender).arg(liu.tvs.size()));
    foreach(TV tv,liu.tvs)
    {
        resultTextEdit->append(tr("电视剧:%1\n主演:%2\n配角:%3\n").arg(tv.TVName).arg(tv.actress.join(",")).arg(tv.actor.join(",")));
        foreach(QString key,tv.boxOffice.keys())
        {
            resultTextEdit->append(tr("票房:%1\n").arg(tv.boxOffice[key]));
        }
    }
}

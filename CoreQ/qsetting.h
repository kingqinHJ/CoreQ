#ifndef QSETTING_H
#define QSETTING_H

#include <QtWidgets>
#include <QSettings>

class QSetting : public QWidget
{
    Q_OBJECT
public:
    explicit QSetting(QWidget *parent = nullptr);

private slots:
    void saveSettings();
    void loadSettings();

private:
    void setupUI();
    void initConnections();

    QSettings *settings;
    QLineEdit *usernameEdit;
    QLineEdit *emailEdit;
    QPushButton *saveButton;
    QPushButton *loadButton;
    QVBoxLayout *mainLayout;
    QFormLayout *formLayout;
};

#endif // QSETTING_H

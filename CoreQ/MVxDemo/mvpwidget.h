#pragma once

#include <QWidget>
#include "SimpleModel.h"

class QSlider;
class QDoubleSpinBox;
class QDial;
class QLabel;
class QPushButton;

/**
 * @brief MVP 示例窗口
 * 
 * 展示基于 LMMS 风格的 "SimpleModel" 架构 (Model-View-Presenter 的变体)。
 * 核心思想：
 * 1. Model (FloatModel) 负责数据存储、范围限制 (Clamp) 和步进对齐 (Step Align)。
 * 2. View (Slider, SpinBox, Dial) 只负责显示和用户输入，不持有业务逻辑。
 * 3. Presenter (本类中的逻辑) 负责将 Model 和 View 绑定，实现多视图同步。
 */
class MVPWidget : public QWidget {
    Q_OBJECT
public:
    explicit MVPWidget(QWidget* parent = nullptr);
    ~MVPWidget() override = default;

private:
    // 初始化 UI
    void initUI();
    // 初始化 Presenter 逻辑 (绑定 Model 与 View)
    void initPresenter();

private:
    // --- Model ---
    // 使用 FloatModel (TypedModel<float>)，统一管理数值逻辑
    FloatModel* m_volumeModel;

    // --- Views ---
    // 多个视图展示同一个数据，测试同步性
    QSlider* m_sliderView;          // 视图1: 滑块
    QDoubleSpinBox* m_spinBoxView;  // 视图2: 数字框
    QDial* m_dialView;              // 视图3: 旋钮
    QLabel* m_displayLabel;         // 视图4: 文本显示
    QPushButton* m_btnReset;        // 视图5: 重置按钮
};

#include "mvpwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QDial>
#include <QPushButton>
#include <QGroupBox>

MVPWidget::MVPWidget(QWidget* parent)
    : QWidget(parent)
    , m_volumeModel(nullptr)
{
    // 1. 创建 Model (业务逻辑层)
    // 参数: 初始值=50.0, 最小值=0.0, 最大值=100.0, 步进=0.5
    // 所有的范围验证、步进对齐都在 Model 内部自动处理，无需 View 操心。
    m_volumeModel = new FloatModel(50.0f, 0.0f, 100.0f, 0.5f, this);

    initUI();
    initPresenter();
}

void MVPWidget::initUI()
{
    auto* mainLayout = new QVBoxLayout(this);

    // --- 标题部分 ---
    auto* title = new QLabel("MVP 架构示例 (Based on LMMS SimpleModel)", this);
    title->setStyleSheet("font-weight: bold; font-size: 16px; color: #2c3e50;");
    auto* desc = new QLabel("核心演示：\n"
                            "1. 多视图同步 (One Model, Multi-Views)\n"
                            "2. 参数自我纠正与验证 (Validation in Model)\n"
                            "3. 解耦 (View 不知晓 Model 逻辑)", this);
    desc->setStyleSheet("color: #7f8c8d; margin-bottom: 10px;");
    desc->setWordWrap(true);
    mainLayout->addWidget(title);
    mainLayout->addWidget(desc);

    // --- 演示区域 ---
    auto* demoGroup = new QGroupBox("音量控制 (Volume Control)", this);
    auto* demoLayout = new QHBoxLayout(demoGroup);

    // View 1: 旋钮 (Dial)
    auto* dialLayout = new QVBoxLayout();
    m_dialView = new QDial(this);
    m_dialView->setRange(0, 100); // 注意：QDial 是整数控件，Presenter 需处理类型转换
    m_dialView->setNotchesVisible(true);
    dialLayout->addWidget(new QLabel("View 1: Dial", this));
    dialLayout->addWidget(m_dialView);
    demoLayout->addLayout(dialLayout);

    // View 2: 滑块 (Slider)
    auto* sliderLayout = new QVBoxLayout();
    m_sliderView = new QSlider(Qt::Vertical, this);
    m_sliderView->setRange(0, 100);
    sliderLayout->addWidget(new QLabel("View 2: Slider", this));
    sliderLayout->addWidget(m_sliderView, 0, Qt::AlignHCenter);
    demoLayout->addLayout(sliderLayout);

    // View 3 & 4 & 5: 详细控制区
    auto* detailLayout = new QVBoxLayout();
    
    // View 3: 数字框 (SpinBox) - 支持浮点
    m_spinBoxView = new QDoubleSpinBox(this);
    m_spinBoxView->setRange(-200.0, 200.0); // 故意设置比 Model 更大的范围，测试 Model 的 Clamp 能力
    m_spinBoxView->setSingleStep(0.5);
    m_spinBoxView->setPrefix("Vol: ");
    m_spinBoxView->setSuffix(" %");
    
    // View 4: 纯文本展示 (Label)
    m_displayLabel = new QLabel("Current: --", this);
    m_displayLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #e74c3c;");

    // View 5: 重置按钮
    m_btnReset = new QPushButton("重置 (Reset)", this);

    detailLayout->addWidget(new QLabel("View 3: SpinBox (Range -200~200)", this));
    detailLayout->addWidget(m_spinBoxView);
    detailLayout->addWidget(new QLabel("View 4: Label (Read-only)", this));
    detailLayout->addWidget(m_displayLabel);
    detailLayout->addStretch();
    detailLayout->addWidget(m_btnReset);
    
    demoLayout->addLayout(detailLayout);
    mainLayout->addWidget(demoGroup);
    mainLayout->addStretch();
}

void MVPWidget::initPresenter()
{
    // ========================================================================
    // Presenter 逻辑 (编排层)
    // 负责将 View 的事件转发给 Model，并将 Model 的变化同步回 View。
    // 实现了 View 和 Model 的彻底解耦。
    // ========================================================================

    // --- 1. Model -> View (数据驱动界面) ---
    // 当 Model 数据变化时，更新所有 View
    connect(m_volumeModel, &SimpleModel::valueChanged, this, [this](float val) {
        // 1. 更新 Dial (需要 float -> int)
        // QDial/QSlider 是整数控件，这里简单的转换可能会丢失精度，
        // 实际项目中可能需要 multiplier (如 value * 100)
        {
            QSignalBlocker blocker(m_dialView); // 防止循环信号 (View -> Model -> View)
            m_dialView->setValue(static_cast<int>(val));
        }

        // 2. 更新 Slider
        {
            QSignalBlocker blocker(m_sliderView);
            m_sliderView->setValue(static_cast<int>(val));
        }

        // 3. 更新 SpinBox
        {
            QSignalBlocker blocker(m_spinBoxView);
            m_spinBoxView->setValue(val);
        }

        // 4. 更新 Label
        m_displayLabel->setText(QString("Current Value: %1").arg(val, 0, 'f', 1));
    });


    // --- 2. View -> Model (界面驱动数据) ---
    // 任何一个 View 的修改都只调用 Model 的 setValue，
    // Model 内部会处理验证，然后发出 valueChanged 信号，再次触发上面的 Model -> View 同步。
    // 这种 "单向数据流" (或闭环同步) 保证了所有 View 永远显示 Model 处理后的合法值。

    // Dial -> Model
    connect(m_dialView, &QDial::valueChanged, this, [this](int val) {
        m_volumeModel->setValue(static_cast<float>(val));
    });

    // Slider -> Model
    connect(m_sliderView, &QSlider::valueChanged, this, [this](int val) {
        m_volumeModel->setValue(static_cast<float>(val));
    });

    // SpinBox -> Model
    connect(m_spinBoxView, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double val) {
        m_volumeModel->setValue(static_cast<float>(val));
        // 演示 Model 的验证能力：
        // 如果用户在 SpinBox 输入 150 (超过 Model max 100)，
        // Model::setValue(150) -> fittedValue(150) -> 100 -> emit valueChanged(100)
        // -> 上面的 Model->View 逻辑会把 SpinBox 设回 100。
        // 结果：SpinBox 会自动跳回 100，实现了"脏数据拒绝"。
    });

    // Reset Button -> Model
    connect(m_btnReset, &QPushButton::clicked, this, [this]() {
        m_volumeModel->reset(); // 恢复初始值 (50.0)
    });


    // --- 3. 初始化同步 ---
    // 手动触发一次，让所有 View 显示初始状态
    emit m_volumeModel->valueChanged(m_volumeModel->value());
}

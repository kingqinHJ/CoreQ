#include "SimpleModel.h"
#include <iostream>
#include <string>
#include <memory>

// ==========================================
// 1. View (视图层) - 模拟
// 模拟一个 UI 滑块，只负责显示和发送用户操作事件
// ==========================================
class MockSlider : public QObject {
    Q_OBJECT
public:
    void setUserValue(int val) {
        std::cout << "[UI Slider] User moved slider to: " << val << std::endl;
        emit valueChanged(val);
    }

    void updateDisplay(int val) {
        std::cout << "[UI Slider] Display updated to: " << val << std::endl;
    }

signals:
    void valueChanged(int val);
};

// ==========================================
// 2. Model (模型层) - 使用我们的 SimpleModel
// ==========================================
// 这里我们可以直接使用 IntModel，或者继承它添加业务逻辑
class VolumeModel : public IntModel {
public:
    VolumeModel(QObject* parent = nullptr)
        : IntModel(50, 0, 100, 1, parent) // Val=50, Min=0, Max=100, Step=1
    {}
};

// ==========================================
// 3. Presenter (编排层)
// 负责将 Model 和 View 绑定在一起
// ==========================================
class VolumePresenter : public QObject {
    Q_OBJECT
public:
    VolumePresenter(VolumeModel* model, MockSlider* view, QObject* parent = nullptr)
        : QObject(parent), m_model(model), m_view(view)
    {
        // A. Model -> View (数据变动驱动 UI 更新)
        connect(m_model, &SimpleModel::valueChanged, this, [view](float newVal){
            // Model 存的是 float (核心)，View 此时需要 int
            view->updateDisplay(static_cast<int>(newVal));
        });

        // B. View -> Model (用户操作驱动数据更新)
        connect(view, &MockSlider::valueChanged, this, [model](int newVal){
            model->setValue(newVal); 
            // Model 会自动处理 Range Clamp 和 Step Align
        });
        
        // C. 初始化视图状态
        view->updateDisplay(m_model->value());
    }

private:
    VolumeModel* m_model;
    MockSlider* m_view;
};

// ==========================================
// Main 模拟运行
// ==========================================
/*
int main() {
    // 1. 创建组件
    VolumeModel model;
    MockSlider view;
    
    // 2. 组装 (Presenter)
    VolumePresenter presenter(&model, &view);

    // 3. 模拟交互
    // 用户试图拖动到 150 (超出 Max 100)
    view.setUserValue(150); 
    // -> View emits 150
    // -> Presenter sets Model 150
    // -> Model clamps to 100
    // -> Model emits 100
    // -> Presenter updates View to 100
    // Output: [UI Slider] Display updated to: 100
    
    return 0;
}
*/

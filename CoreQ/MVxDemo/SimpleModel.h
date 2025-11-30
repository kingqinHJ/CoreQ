#ifndef SIMPLE_MODEL_H
#define SIMPLE_MODEL_H

#include <QObject>
#include <cmath>
#include <algorithm>
#include <vector>
#include <iostream>

// 1. 基础模型类 (Base Model)
// 负责管理核心数据(float)、范围限制、步进、初始值，并提供信号机制。
// 剥离了 LMMS 特定的 JournallingObject (Undo/Redo), ControllerConnection 等。
class SimpleModel : public QObject
{
    Q_OBJECT
public:
    explicit SimpleModel(float val = 0.0f, float min = 0.0f, float max = 1.0f, float step = 0.0f, QObject* parent = nullptr)
        : QObject(parent)
        , m_value(val)
        , m_initValue(val)
        , m_minValue(min)
        , m_maxValue(max)
        , m_step(step)
    {
        // 构造时确保初始值在范围内
        m_value = fittedValue(m_value);
    }

    virtual ~SimpleModel() = default;

    // --- Getters ---
    float value() const { return m_value; }
    float initValue() const { return m_initValue; }
    float minValue() const { return m_minValue; }
    float maxValue() const { return m_maxValue; }
    float step() const { return m_step; }

    // --- Setters ---
    void setValue(float val) {
        float newVal = fittedValue(val);
        if (std::abs(newVal - m_value) > 0.00001f) { //简单的浮点比较
            m_value = newVal;
            emit valueChanged(m_value);
        }
    }

    void setRange(float min, float max, float step = 0.0f) {
        m_minValue = min;
        m_maxValue = max;
        m_step = step;
        // 范围改变可能导致当前值越界，需要重新fit
        setValue(m_value); 
        emit rangeChanged();
    }

signals:
    void valueChanged(float newValue);
    void rangeChanged();

protected:
    // 核心逻辑：将输入值限制在 Min-Max 之间，并按 Step 对齐
    float fittedValue(float val) const {
        // 1. Clamp
        if (val < m_minValue) val = m_minValue;
        if (val > m_maxValue) val = m_maxValue;

        // 2. Step align (if step > 0)
        if (m_step > 0.0f) {
            // 简单的对齐算法: value = min + round((val - min) / step) * step
            float steps = std::round((val - m_minValue) / m_step);
            val = m_minValue + steps * m_step;
            
            // 二次Clamp防止精度误差导致越界
            if (val < m_minValue) val = m_minValue;
            if (val > m_maxValue) val = m_maxValue;
        }
        return val;
    }

private:
    float m_value;
    float m_initValue;
    float m_minValue;
    float m_maxValue;
    float m_step;
};

// 2. 泛型模板类 (Typed Wrapper)
// 提供类型安全访问 (int, bool, float 等)，模仿 TypedAutomatableModel
template <typename T>
class TypedModel : public SimpleModel
{
public:
    TypedModel(T val, T min, T max, T step, QObject* parent = nullptr)
        : SimpleModel(static_cast<float>(val), 
                      static_cast<float>(min), 
                      static_cast<float>(max), 
                      static_cast<float>(step), 
                      parent)
    {}

    // 类型安全的 Getter
    T value() const {
        return static_cast<T>(SimpleModel::value());
    }

    // 类型安全的 Setter
    void setValue(T val) {
        SimpleModel::setValue(static_cast<float>(val));
    }
    
    // 辅助方法：重置为初始值
    void reset() {
        setValue(static_cast<T>(SimpleModel::initValue()));
    }
};

// 3. 具体的常用类型别名 (Aliases)
using FloatModel = TypedModel<float>;
using IntModel   = TypedModel<int>;
using BoolModel  = TypedModel<bool>; // bool 通常 min=0, max=1, step=1

#endif // SIMPLE_MODEL_H

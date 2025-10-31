#ifndef UI_CORE_CONTROL_LOADING_H_
#define UI_CORE_CONTROL_LOADING_H_

#include "duilib/Core/UiTypes.h"
#include "duilib/Core/ControlPtrT.h"
#include <memory>
#include <chrono>

namespace ui 
{
/** 控件加载中状态的逻辑封装
*/
class Control;
class IRender;
class Box;
class LoadingAttribute;

//控件的loading状态
struct ControlLoadingStatus
{
    //该loading状态关联的控件接口
    ControlPtrT<Control> m_pControl;

    //该Loading状态界面显示的根容器接口，可用根据该接口操作界面元素
    ControlPtrT<Box> m_pLoadingUiRootBox;

    //loading开始到现在的时间（毫秒）
    uint64_t m_nElapsedTimeMs = 0;

    //动画图片所在的控件接口(需要正确配置XML属性)
    //手动播放动画的情况，可通过该接口的SetImageAnimationFrame来控制动画播放到哪一帧
    ControlPtrT<Control> m_pAnimationControl;

    //返回值：如果返回true表示需要终止loading状态，否则继续处于loading状态(仅在kEventLoading事件中有效)
    bool m_bStopLoading = false;
};

class UILIB_API ControlLoading: public SupportWeakCallback
{
public:
    explicit ControlLoading(Control* pControl);
    virtual ~ControlLoading() override;
    ControlLoading(const ControlLoading&) = delete;
    ControlLoading& operator = (const ControlLoading&) = delete;

public:
    /** 设置loading的属性，根据属性中指定的XML文件创建显示界面，并设置界面属性
    * @param [in] loadingAttribute loading的属性字符串
    */
    bool SetLoadingAttribute(const DString& loadingAttribute);

    /** 绘制"加载中"的状态
    */
    void PaintLoading(IRender* pRender, const UiRect& rcPaint);

    /** 开启loading状态
    * @param [in] nIntervalMs 回调的时间间隔（毫秒），最小值为10毫秒
    * @param [in] nMaxCount 设置最大回调次数，超过此值则自动停止，loading总时间为 nMaxCount * nIntervalMs 毫秒；如果为-1表示一值加载，等待手动停止
    */
    bool StartLoading(int32_t nIntervalMs, int32_t nMaxCount);

    /** 关闭loading状态
    */
    void StopLoading();

    /** 是否正在loading
    * @return 在loading返回true, 反之返回false
    */
    bool IsLoading() const;

    /** 更新loading控件的位置
    */
    void UpdateLoadingPos();

private:
    /** 计算loading图片的旋转角度, 旋转图片
    */
    void Loading();

    /** 解析loading属性
    */
    bool InitAttribute(LoadingAttribute& loadingAttribute, const DString& loadingString);

private:
    //Loading的属性
    std::unique_ptr<LoadingAttribute> m_pLoadingAttribute;

    //加载中状态的生命周期管理、取消机制
    WeakCallbackFlag m_loadingFlag;

    //关联的Control对象
    Control* m_pControl;

    //Loading界面的容器
    std::unique_ptr<Box> m_pLoadingBox;

    //动画图片所在的控件接口(需要正确配置XML属性)
    //手动播放动画的情况，可通过该接口的SetImageAnimationFrame来控制动画播放到哪一帧
    Control* m_pAnimationControl;

    //开始时间
    std::chrono::steady_clock::time_point m_startTime;

    //加载中回调函数的调用时间周期
    int32_t m_nIntervalMs;

    //最大回调次数，如果为-1表示一直加载中，等待手动停止
    int32_t m_nMaxCount;

    //当前加载回调次数
    int32_t m_nCallbackCount;

    //是否处于加载中的状态
    bool m_bIsLoading;
};

} // namespace ui

#endif // UI_CORE_CONTROL_LOADING_H_

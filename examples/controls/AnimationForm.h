#ifndef EXAMPLES_ANIMATION_FORM_H_
#define EXAMPLES_ANIMATION_FORM_H_

// duilib
#include "duilib/duilib.h"

#include <vector>

class AnimationForm : public ui::WindowImplBase
{
public:
    AnimationForm();
    virtual ~AnimationForm() override;

    /** 资源相关接口
     * GetSkinFolder 接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile 接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 语言切换事件：刷新动态生成的文本
    */
    virtual bool OnLanguageChanged() override;

private:
    /** 测试图片的定义
    */
    struct TestImageItem
    {
        //图片文件名（相对于render目录）
        DString m_strFileName;

        //图片显示名称的语言资源ID
        DString m_strNameId;

        //ICO文件是否按多帧动画加载
        bool m_bIconAsAnimation = false;

        //ICO文件加载时的图片大小
        uint32_t m_nIconSize = 0;
    };

private:
    /** 初始化测试图片列表（下拉框）
    */
    void InitImageList();

    /** 初始化播放控制按钮
    */
    void InitPlayControls();

    /** 按当前参数加载/重新加载当前选中的图片
    */
    void ApplyImage();

    /** 生成当前图片的bkimage属性串
    */
    DString BuildImageString() const;

    /** 从界面上读取“自动播放”参数
    */
    bool GetAutoPlay() const;

    /** 从界面上读取“播放次数”参数（-1表示一直播放）
    */
    int32_t GetPlayCount() const;

    /** 播放动画
    * @param [in] nStartFrame 从哪一帧开始播放
    */
    void StartImageAnimation(ui::AnimationImagePos nStartFrame);

    /** 停止动画
    * @param [in] nStopFrame 停止在哪一帧
    * @param [in] bTriggerEvent 是否触发停止事件
    */
    void StopImageAnimation(ui::AnimationImagePos nStopFrame, bool bTriggerEvent);

    /** 向前/向后切换一帧（负数表示向前，正数表示向后）
    */
    void StepImageFrame(int32_t nStep);

    /** 跳转到指定帧
    */
    void GotoImageFrame();

    /** 更新播放状态（按钮文字、内部标记）
    */
    void UpdatePlayState(bool bPlaying);

    /** 更新“当前帧/总帧数”的显示
    * @param [in] nFrameDelayMs 当前帧的播放间隔(毫秒)，小于0表示不显示
    */
    void UpdateFrameLabel(int32_t nFrameDelayMs = -1);

    /** 更新图片信息（尺寸、帧数、循环次数、帧间隔）的显示
    */
    void UpdateImageInfo();

    /** 更新事件统计的显示
    */
    void UpdateEventInfo();

    /** 更新“跳至帧”输入框的取值范围
    */
    void UpdateGotoFrameRange();

    /** 获取图片显示控件
    */
    ui::Control* GetImageControl() const;

    /** 获取图片显示控件中动画图片对象
    */
    ui::Image* GetDuiImage() const;

private:
    //动画播放相关事件处理
    void OnAnimationEvents(ui::EventType eventType, const ui::ImageAnimationStatus& status);

private:
    //测试图片列表
    std::vector<TestImageItem> m_imageList;

    //当前选中的图片索引号
    size_t m_nImageIndex;

    //是否正在播放中
    bool m_bImagePlaying;

    //当前帧索引号
    int32_t m_nFrameIndex;

    //图片总帧数
    int32_t m_nFrameCount;

    //播放开始事件的次数
    int32_t m_nStartEventCount;

    //播放停止事件的次数
    int32_t m_nStopEventCount;

    //播放帧事件的次数
    int32_t m_nFrameEventCount;
};

#endif //EXAMPLES_ANIMATION_FORM_H_

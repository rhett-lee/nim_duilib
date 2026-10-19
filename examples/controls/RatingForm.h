#ifndef EXAMPLES_CONTROLS_RATING_FORM_H_
#define EXAMPLES_CONTROLS_RATING_FORM_H_

#include "duilib/duilib.h"

/** Rating 星级评分控件演示窗口
*/
class RatingForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    RatingForm();
    virtual ~RatingForm() override;

    /** 加载资源文件的目录，位于 bin/resources/ 目录下
    */
    virtual DString GetSkinFolder() override;

    /** 窗口使用的 XML 布局文件
    */
    virtual DString GetSkinFile() override;

    /** 窗口初始化
    */
    virtual void OnInitWindow() override;

private:
    /** 把评分值回显到标签
    * @param [in] pRating 评分控件
    */
    void UpdateRatingText(ui::Rating* pRating);
};

#endif // EXAMPLES_CONTROLS_RATING_FORM_H_

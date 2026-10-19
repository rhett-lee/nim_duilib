#ifndef EXAMPLES_CONTROLS_CAROUSEL_FORM_H_
#define EXAMPLES_CONTROLS_CAROUSEL_FORM_H_

#include "duilib/duilib.h"

/** Carousel 图片轮播 / Pagination 分页器控件演示窗口
*/
class CarouselForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    CarouselForm();
    virtual ~CarouselForm() override;

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
    /** 把轮播当前索引回显到标签
    * @param [in] pCarousel 轮播控件
    */
    void UpdateCarouselText(ui::Carousel* pCarousel);

    /** 把分页器当前页码回显到标签
    * @param [in] pPagination 分页器控件
    */
    void UpdatePaginationText(ui::Pagination* pPagination);
};

#endif // EXAMPLES_CONTROLS_CAROUSEL_FORM_H_

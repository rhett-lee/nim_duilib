#ifndef UI_COMPONENTS_MSGBOX_H_
#define UI_COMPONENTS_MSGBOX_H_

#include "duilib/Utils/WinImplBase.h"
#include "duilib/Control/Button.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Control/Label.h"
#include <functional>

namespace nim_comp {

enum MsgBoxRet
{
    MB_YES,
    MB_NO
};

typedef std::function<void(MsgBoxRet)> MsgboxCallback;

void ShowMsgBox(ui::Window* pWindow, MsgboxCallback cb,
                const DString& content = _T(""), bool content_is_id = true,
                const DString& title = _T("STRING_TIPS"), bool title_is_id = true,
                const DString& yes = _T("STRING_OK"), bool btn_yes_is_id = true,
                const DString& no = _T(""), bool btn_no_is_id = false);

class MsgBox : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    friend void ShowMsgBox(ui::Window* pWindow, MsgboxCallback cb,
                           const DString& content, bool content_is_id,
                           const DString& title, bool title_is_id,
                           const DString& yes, bool btn_yes_is_id,
                           const DString& no, bool btn_no_is_id);
public:
    MsgBox();
    virtual ~MsgBox() override;

    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;
    virtual DString GetZIPFileName() const;
    virtual void OnEsc(BOOL &bHandled);

    virtual DString GetWindowId() const /*override*/;
    virtual void OnInitWindow() override;
    virtual void OnCloseWindow() override;
private:
    bool OnClicked(const ui::EventArgs& msg);

    void SetTitle(const DString &str);
    void SetContent(const DString &str);
    void SetButton(const DString &yes, const DString &no);
    void Show(ui::Window* pParentWindow, MsgboxCallback cb);

    void EndMsgBox(MsgBoxRet ret);
public:
    static const DString kClassName;
private:
    ui::Label* m_title   = nullptr;
    ui::RichEdit* m_content = nullptr;
    ui::Button* m_btn_yes = nullptr;
    ui::Button* m_btn_no  = nullptr;

    MsgboxCallback m_msgbox_callback;
    ui::Window* m_pParentWindow = nullptr;
};

}

#endif //UI_COMPONENTS_MSGBOX_H_

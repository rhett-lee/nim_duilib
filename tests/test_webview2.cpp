// WebView2 目录的单元测试
//
// 测试对象（不启动 WebView2 运行时、不创建窗口、不下载网页）：
//  1. ComPtr：引用计数管理、指针接管、拷贝/移动、Reset/Detach/Swap、AsIID 查询；
//  2. ComCallback：回调对象的创建（引用计数不能多1，否则泄漏）、Invoke 的0/1/2参数
//     调用、成员函数形式的回调、QueryInterface 语义；
//  3. WebView2EnvironmentOptions：创建实例、IUnknown/各个扩展接口的 QueryInterface、
//     字符串属性（CoTaskMemAlloc 输出）与布尔/枚举属性的默认值和读写；
//  4. WebView2Manager：默认配置、各配置项的读写、语言（含设置语言后的优先级）、
//     默认缓存目录、Initialize 注册控件创建回调；
//  5. WebView2Control：控件类型、未初始化时的默认行为与状态查询、
//     初始URL的拼接规则、未初始化时接口调用的返回值。

#if defined(_WIN32)
#include <crtdbg.h>
#endif

#include "duilib/WebView2/ComPtr.h"
#include "duilib/WebView2/ComCallback.h"
#include "duilib/WebView2/WebView2EnvironmentOptions.h"
#include "duilib/WebView2/WebView2Manager.h"
#include "duilib/WebView2/WebView2Control.h"

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/StringUtil.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace
{
int32_t g_failedCount = 0;
int32_t g_checkCount = 0;

void Check(bool condition, const char* expression)
{
    ++g_checkCount;
    if (!condition) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << expression << std::endl;
    }
}

#define CHECK(condition) Check((condition), #condition)

void CheckInt(int64_t actual, int64_t expected, const char* expression)
{
    ++g_checkCount;
    if (actual != expected) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << expression << ": actual=" << actual
                  << ", expected=" << expected << std::endl;
    }
}

#define CHECK_INT(actual, expected) CheckInt((int64_t)(actual), (int64_t)(expected), #actual " == " #expected)
#define CHECK_HR(actual, expected) CheckInt((int64_t)(actual), (int64_t)(expected), #actual " == " #expected)

void PrintString(const DString& text)
{
#ifdef DUILIB_UNICODE
    std::wcerr << text;
#else
    std::cerr << text;
#endif
}

void CheckStr(const DString& actual, const DString& expected, const char* expression)
{
    ++g_checkCount;
    if (actual != expected) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << expression << ": actual=\"";
        PrintString(actual);
        std::cerr << "\", expected=\"";
        PrintString(expected);
        std::cerr << "\"" << std::endl;
    }
}

#define CHECK_STR(actual, expected) CheckStr((actual), (expected), #actual " == " #expected)

/** 输出当前测试小节的标题，便于定位失败用例 */
void Section(const char* name)
{
    std::cerr << "--- " << name << std::endl;
}

/** 测试用IID：仅作为模板参数使用，不需要注册到COM */
constexpr IID kIIDTestCallback0 = { 0x9B7E5C01, 0x1111, 0x4A01,
                                    { 0x8E, 0x01, 0x00, 0x11, 0x22, 0x33, 0x44, 0x01 } };
constexpr IID kIIDTestCallback1 = { 0x9B7E5C02, 0x1111, 0x4A01,
                                    { 0x8E, 0x01, 0x00, 0x11, 0x22, 0x33, 0x44, 0x02 } };
constexpr IID kIIDTestCallback2 = { 0x9B7E5C03, 0x1111, 0x4A01,
                                    { 0x8E, 0x01, 0x00, 0x11, 0x22, 0x33, 0x44, 0x03 } };
constexpr IID kIIDUnknown      = { 0x9B7E5CFF, 0x1111, 0x4A01,
                                    { 0x8E, 0x01, 0x00, 0x11, 0x22, 0x33, 0x44, 0xFF } };

/** 测试用的最小COM对象：引用计数从1开始（等价于刚new出来的COM对象） */
class TestComObject
{
public:
    TestComObject() { ++s_liveCount; }
    virtual ~TestComObject() { --s_liveCount; }

    LONG AddRef()
    {
        return ++m_refCount;
    }

    LONG Release()
    {
        LONG refCount = --m_refCount;
        if (refCount == 0) {
            delete this;
        }
        return refCount;
    }

    HRESULT QueryInterface(REFIID /*riid*/, void** ppvObject)
    {
        if (ppvObject == nullptr) {
            return E_POINTER;
        }
        *ppvObject = this;
        AddRef();
        return S_OK;
    }

    LONG m_refCount = 1;
    static int32_t s_liveCount;
};

int32_t TestComObject::s_liveCount = 0;

/** 测试用的回调接口：0个参数 */
struct ITestCallback0 : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE Invoke() = 0;
};

/** 测试用的回调接口：1个参数 */
struct ITestCallback1 : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE Invoke(int value) = 0;
};

/** 测试用的回调接口：2个参数 */
struct ITestCallback2 : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE Invoke(int value, PCWSTR text) = 0;
};

/** 测试用成员函数回调的宿主类 */
class TestCallbackHandler
{
public:
    HRESULT STDMETHODCALLTYPE OnInvoke(int value)
    {
        m_value = value;
        return S_OK;
    }

    int m_value = 0;
};

/** ComPtr 的引用计数与所有权语义 */
void TestComPtr()
{
    Section("ComPtr");

    CHECK_INT(TestComObject::s_liveCount, 0);
    {
        ui::ComPtr<TestComObject> spObject;
        CHECK(spObject == nullptr);
        CHECK(spObject.Get() == nullptr);
        CHECK(!spObject);

        //接管刚创建的对象（引用计数已经是1），不增加引用计数
        *spObject.GetAddressOf() = new TestComObject;
        CHECK(spObject != nullptr);
        CHECK_INT(spObject.Get()->m_refCount, 1);
        CHECK_INT(TestComObject::s_liveCount, 1);

        //GetAddressOf 会先释放旧对象
        TestComObject* pOld = spObject.Get();
        *spObject.GetAddressOf() = new TestComObject;
        CHECK(spObject.Get() != pOld);
        CHECK_INT(TestComObject::s_liveCount, 1);
        CHECK_INT(spObject.Get()->m_refCount, 1);

        //拷贝构造：增加引用计数
        {
            ui::ComPtr<TestComObject> spCopy(spObject);
            CHECK(spCopy.Get() == spObject.Get());
            CHECK_INT(spObject.Get()->m_refCount, 2);

            //移动构造：转移所有权，源对象置空
            ui::ComPtr<TestComObject> spMove(std::move(spCopy));
            CHECK(spCopy == nullptr);
            CHECK(spMove.Get() == spObject.Get());
            CHECK_INT(spObject.Get()->m_refCount, 2);
        }
        CHECK_INT(spObject.Get()->m_refCount, 1);

        //从原始指针构造：增加引用计数（调用方需要释放自己的引用）
        TestComObject* pRaw = new TestComObject;
        {
            ui::ComPtr<TestComObject> spFromRaw(pRaw);
            CHECK_INT(pRaw->m_refCount, 2);
        }
        CHECK_INT(pRaw->m_refCount, 1);
        pRaw->Release();
        CHECK_INT(TestComObject::s_liveCount, 1);

        //Detach：放弃所有权，不减少引用计数
        TestComObject* pDetached = spObject.Detach();
        CHECK(spObject == nullptr);
        CHECK(pDetached != nullptr);
        CHECK_INT(pDetached->m_refCount, 1);
        pDetached->Release();
        CHECK_INT(TestComObject::s_liveCount, 0);

        //Reset(T*)：增加一次引用计数（调用方需要释放自己的引用）
        TestComObject* pReset = new TestComObject;
        spObject.Reset(pReset);
        CHECK(spObject.Get() == pReset);
        CHECK_INT(pReset->m_refCount, 2);
        pReset->Release();
        CHECK_INT(spObject.Get()->m_refCount, 1);
    }
    CHECK_INT(TestComObject::s_liveCount, 0);

    //Swap
    {
        ui::ComPtr<TestComObject> spFirst;
        ui::ComPtr<TestComObject> spSecond;
        *spFirst.GetAddressOf() = new TestComObject;
        *spSecond.GetAddressOf() = new TestComObject;
        TestComObject* pFirst = spFirst.Get();
        TestComObject* pSecond = spSecond.Get();
        spFirst.Swap(spSecond);
        CHECK(spFirst.Get() == pSecond);
        CHECK(spSecond.Get() == pFirst);
    }
    CHECK_INT(TestComObject::s_liveCount, 0);

    //AsIID：查询结果通过输出参数返回，并增加一次引用计数
    {
        ui::ComPtr<TestComObject> spObject;
        *spObject.GetAddressOf() = new TestComObject;

        void* pOut = nullptr;
        CHECK_HR(spObject.AsIID<TestComObject>(IID_IUnknown, &pOut), S_OK);
        CHECK(pOut != nullptr);
        CHECK_INT(spObject.Get()->m_refCount, 2);
        if (pOut != nullptr) {
            static_cast<TestComObject*>(pOut)->Release();
        }
        CHECK_INT(spObject.Get()->m_refCount, 1);

        //空输出参数
        CHECK_HR(spObject.AsIID<TestComObject>(IID_IUnknown, nullptr), E_POINTER);

        //空对象查询
        ui::ComPtr<TestComObject> spEmpty;
        pOut = reinterpret_cast<void*>(1);
        CHECK_HR(spEmpty.AsIID<TestComObject>(IID_IUnknown, &pOut), E_NOINTERFACE);
        CHECK(pOut == nullptr);
    }
    CHECK_INT(TestComObject::s_liveCount, 0);

    //MakeComPtr：引用计数必须为1，否则对象永远不会释放
    {
        ui::ComPtr<TestComObject> spObject = ui::MakeComPtr<TestComObject>();
        CHECK(spObject != nullptr);
        CHECK_INT(spObject.Get()->m_refCount, 1);
        CHECK_INT(TestComObject::s_liveCount, 1);
    }
    CHECK_INT(TestComObject::s_liveCount, 0);
}

/** ComCallback 的回调对象创建与调用语义 */
void TestComCallback()
{
    Section("ComCallback");

    {
        //0个参数：验证回调被调用
        int callCount = 0;
        {
            ui::ComPtr<ITestCallback0> spCallback =
                ui::ComCallback<ITestCallback0, kIIDTestCallback0>(
                    [&callCount]() -> HRESULT {
                        ++callCount;
                        return S_OK;
                    });
            CHECK(spCallback != nullptr);
            CHECK_INT(spCallback->Invoke(), S_OK);
            CHECK_INT(callCount, 1);
        }
        CHECK_INT(callCount, 1);
    }

    {
        //引用计数不能多1：回调对象销毁时，其捕获的资源必须一起释放
        std::shared_ptr<int> token = std::make_shared<int>(1);
        std::weak_ptr<int> weakToken = token;
        {
            ui::ComPtr<ITestCallback0> spCallback =
                ui::ComCallback<ITestCallback0, kIIDTestCallback0>(
                    [token]() -> HRESULT {
                        return S_OK;
                    });
            CHECK(spCallback != nullptr);
            token.reset();
            //回调对象存活期间，持有捕获的shared_ptr
            CHECK(!weakToken.expired());
        }
        //回调对象已释放，说明引用计数没有多1
        CHECK(weakToken.expired());
    }

    {
        //1个参数：返回值和参数传递
        int received = 0;
        ui::ComPtr<ITestCallback1> spCallback =
            ui::ComCallback<ITestCallback1, kIIDTestCallback1>(
                [&received](int value) -> HRESULT {
                    received = value;
                    return S_FALSE;
                });
        CHECK(spCallback != nullptr);
        CHECK_INT(spCallback->Invoke(42), S_FALSE);
        CHECK_INT(received, 42);
    }

    {
        //2个参数
        int receivedValue = 0;
        DString receivedText;
        ui::ComPtr<ITestCallback2> spCallback =
            ui::ComCallback<ITestCallback2, kIIDTestCallback2>(
                [&receivedValue, &receivedText](int value, PCWSTR text) -> HRESULT {
                    receivedValue = value;
                    receivedText = (text != nullptr) ? text : _T("");
                    return S_OK;
                });
        CHECK(spCallback != nullptr);
        CHECK_INT(spCallback->Invoke(7, L"hello"), S_OK);
        CHECK_INT(receivedValue, 7);
        CHECK_STR(receivedText, _T("hello"));
    }

    {
        //成员函数形式的回调
        TestCallbackHandler handler;
        ui::ComPtr<ITestCallback1> spCallback =
            ui::ComCallback<ITestCallback1, kIIDTestCallback1>(&handler, &TestCallbackHandler::OnInvoke);
        CHECK(spCallback != nullptr);
        CHECK_INT(spCallback->Invoke(9), S_OK);
        CHECK_INT(handler.m_value, 9);
    }

    {
        //QueryInterface：已知接口返回S_OK，未知接口返回E_NOINTERFACE
        ui::ComPtr<ITestCallback0> spCallback =
            ui::ComCallback<ITestCallback0, kIIDTestCallback0>([]() -> HRESULT { return S_OK; });

        IUnknown* pUnknown = nullptr;
        CHECK_HR(spCallback->QueryInterface(IID_IUnknown, (void**)&pUnknown), S_OK);
        CHECK(pUnknown != nullptr);
        if (pUnknown != nullptr) {
            pUnknown->Release();
        }

        IUnknown* pUnknown2 = nullptr;
        CHECK_HR(spCallback->QueryInterface(kIIDUnknown, (void**)&pUnknown2), E_NOINTERFACE);
        CHECK(pUnknown2 == nullptr);

        //AddRef/Release 配对后，回调对象仍然可用
        spCallback->AddRef();
        spCallback->Release();
        CHECK_INT(spCallback->Invoke(), S_OK);
    }
}

/** 读取CoTaskMemAlloc分配的字符串，并释放内存 */
std::wstring TakeWideString(LPWSTR value)
{
    std::wstring result;
    if (value != nullptr) {
        result = value;
        ::CoTaskMemFree(value);
    }
    return result;
}

void CheckWideString(const std::wstring& actual, const std::wstring& expected, const char* expression)
{
    ++g_checkCount;
    if (actual != expected) {
        ++g_failedCount;
        std::wcerr << "[FAIL] " << expression << ": actual=\"" << actual
                   << "\", expected=\"" << expected << "\"" << std::endl;
    }
}

#define CHECK_WSTR(actual, expected) CheckWideString((actual), (expected), #actual " == " #expected)

/** WebView2EnvironmentOptions 的接口与属性 */
void TestEnvironmentOptions()
{
    Section("WebView2EnvironmentOptions");

    //参数校验
    CHECK_HR(ui::WebView2EnvironmentOptions::CreateInstance(nullptr), E_POINTER);

    ui::ComPtr<ICoreWebView2EnvironmentOptions> spOptions;
    CHECK_HR(ui::WebView2EnvironmentOptions::CreateInstance(&spOptions), S_OK);
    CHECK(spOptions != nullptr);
    if (spOptions == nullptr) {
        return;
    }

    //字符串属性的默认值
    LPWSTR value = nullptr;
    CHECK_HR(spOptions->get_AdditionalBrowserArguments(&value), S_OK);
    CHECK(value != nullptr);
    CHECK_WSTR(TakeWideString(value), L"");

    value = nullptr;
    CHECK_HR(spOptions->get_Language(&value), S_OK);
    CHECK(value != nullptr);
    CHECK_WSTR(TakeWideString(value), L"");

    //默认的浏览器版本
    value = nullptr;
    CHECK_HR(spOptions->get_TargetCompatibleBrowserVersion(&value), S_OK);
    CHECK(value != nullptr);
    CHECK_WSTR(TakeWideString(value), CORE_WEBVIEW_TARGET_PRODUCT_VERSION);

    //空指针参数校验
    CHECK_HR(spOptions->get_AdditionalBrowserArguments(nullptr), E_POINTER);
    CHECK_HR(spOptions->get_Language(nullptr), E_POINTER);
    CHECK_HR(spOptions->get_TargetCompatibleBrowserVersion(nullptr), E_POINTER);

    //字符串属性读写
    CHECK_HR(spOptions->put_AdditionalBrowserArguments(L"--disable-gpu"), S_OK);
    value = nullptr;
    CHECK_HR(spOptions->get_AdditionalBrowserArguments(&value), S_OK);
    CHECK_WSTR(TakeWideString(value), L"--disable-gpu");

    CHECK_HR(spOptions->put_Language(L"zh-CN"), S_OK);
    value = nullptr;
    CHECK_HR(spOptions->get_Language(&value), S_OK);
    CHECK_WSTR(TakeWideString(value), L"zh-CN");

    CHECK_HR(spOptions->put_TargetCompatibleBrowserVersion(L"120.0.0.0"), S_OK);
    value = nullptr;
    CHECK_HR(spOptions->get_TargetCompatibleBrowserVersion(&value), S_OK);
    CHECK_WSTR(TakeWideString(value), L"120.0.0.0");

    //把字符串属性重置为空（空指针等价于空字符串）
    CHECK_HR(spOptions->put_AdditionalBrowserArguments(nullptr), S_OK);
    value = nullptr;
    CHECK_HR(spOptions->get_AdditionalBrowserArguments(&value), S_OK);
    CHECK_WSTR(TakeWideString(value), L"");

    //布尔属性的默认值与读写
    BOOL boolValue = TRUE;
    CHECK_HR(spOptions->get_AllowSingleSignOnUsingOSPrimaryAccount(&boolValue), S_OK);
    CHECK_INT(boolValue, FALSE);
    CHECK_HR(spOptions->put_AllowSingleSignOnUsingOSPrimaryAccount(TRUE), S_OK);
    CHECK_HR(spOptions->get_AllowSingleSignOnUsingOSPrimaryAccount(&boolValue), S_OK);
    CHECK_INT(boolValue, TRUE);
    CHECK_HR(spOptions->get_AllowSingleSignOnUsingOSPrimaryAccount(nullptr), E_POINTER);

    //IUnknown与各个扩展接口的QueryInterface
    IUnknown* pUnknown = nullptr;
    CHECK_HR(spOptions->QueryInterface(IID_IUnknown, (void**)&pUnknown), S_OK);
    CHECK(pUnknown != nullptr);
    if (pUnknown != nullptr) {
        pUnknown->Release();
    }

    ui::ComPtr<ICoreWebView2EnvironmentOptions2> spOptions2;
    CHECK_HR(spOptions->QueryInterface(IID_ICoreWebView2EnvironmentOptions2, (void**)&spOptions2), S_OK);
    if (spOptions2 != nullptr) {
        CHECK_HR(spOptions2->get_ExclusiveUserDataFolderAccess(&boolValue), S_OK);
        CHECK_INT(boolValue, FALSE);
        CHECK_HR(spOptions2->put_ExclusiveUserDataFolderAccess(TRUE), S_OK);
        CHECK_HR(spOptions2->get_ExclusiveUserDataFolderAccess(&boolValue), S_OK);
        CHECK_INT(boolValue, TRUE);
    }

    ui::ComPtr<ICoreWebView2EnvironmentOptions3> spOptions3;
    CHECK_HR(spOptions->QueryInterface(IID_ICoreWebView2EnvironmentOptions3, (void**)&spOptions3), S_OK);
    if (spOptions3 != nullptr) {
        CHECK_HR(spOptions3->put_IsCustomCrashReportingEnabled(TRUE), S_OK);
        CHECK_HR(spOptions3->get_IsCustomCrashReportingEnabled(&boolValue), S_OK);
        CHECK_INT(boolValue, TRUE);
    }

    ui::ComPtr<ICoreWebView2EnvironmentOptions4> spOptions4;
    CHECK_HR(spOptions->QueryInterface(IID_ICoreWebView2EnvironmentOptions4, (void**)&spOptions4), S_OK);
    if (spOptions4 != nullptr) {
        //未设置自定义协议时，数量为0，数组为空
        UINT32 count = 1;
        ICoreWebView2CustomSchemeRegistration** registrations =
            reinterpret_cast<ICoreWebView2CustomSchemeRegistration**>(static_cast<size_t>(1));
        CHECK_HR(spOptions4->GetCustomSchemeRegistrations(&count, &registrations), S_OK);
        CHECK_INT(count, 0);
        CHECK(registrations == nullptr);

        //参数校验
        CHECK_HR(spOptions4->GetCustomSchemeRegistrations(nullptr, &registrations), E_POINTER);
        CHECK_HR(spOptions4->GetCustomSchemeRegistrations(&count, nullptr), E_POINTER);

        //设置空列表（数组指针为空时忽略所有元素）
        CHECK_HR(spOptions4->SetCustomSchemeRegistrations(2, nullptr), S_OK);
        count = 1;
        registrations = reinterpret_cast<ICoreWebView2CustomSchemeRegistration**>(static_cast<size_t>(1));
        CHECK_HR(spOptions4->GetCustomSchemeRegistrations(&count, &registrations), S_OK);
        CHECK_INT(count, 0);
        CHECK(registrations == nullptr);
    }

    ui::ComPtr<ICoreWebView2EnvironmentOptions5> spOptions5;
    CHECK_HR(spOptions->QueryInterface(IID_ICoreWebView2EnvironmentOptions5, (void**)&spOptions5), S_OK);
    if (spOptions5 != nullptr) {
        //默认启用跟踪防护
        CHECK_HR(spOptions5->get_EnableTrackingPrevention(&boolValue), S_OK);
        CHECK_INT(boolValue, TRUE);
        CHECK_HR(spOptions5->put_EnableTrackingPrevention(FALSE), S_OK);
        CHECK_HR(spOptions5->get_EnableTrackingPrevention(&boolValue), S_OK);
        CHECK_INT(boolValue, FALSE);
    }

    ui::ComPtr<ICoreWebView2EnvironmentOptions6> spOptions6;
    CHECK_HR(spOptions->QueryInterface(IID_ICoreWebView2EnvironmentOptions6, (void**)&spOptions6), S_OK);
    if (spOptions6 != nullptr) {
        CHECK_HR(spOptions6->get_AreBrowserExtensionsEnabled(&boolValue), S_OK);
        CHECK_INT(boolValue, FALSE);
        CHECK_HR(spOptions6->put_AreBrowserExtensionsEnabled(TRUE), S_OK);
        CHECK_HR(spOptions6->get_AreBrowserExtensionsEnabled(&boolValue), S_OK);
        CHECK_INT(boolValue, TRUE);
    }

    ui::ComPtr<ICoreWebView2EnvironmentOptions7> spOptions7;
    CHECK_HR(spOptions->QueryInterface(IID_ICoreWebView2EnvironmentOptions7, (void**)&spOptions7), S_OK);
    if (spOptions7 != nullptr) {
        COREWEBVIEW2_CHANNEL_SEARCH_KIND channelSearchKind = COREWEBVIEW2_CHANNEL_SEARCH_KIND_LEAST_STABLE;
        CHECK_HR(spOptions7->get_ChannelSearchKind(&channelSearchKind), S_OK);
        CHECK_INT(channelSearchKind, COREWEBVIEW2_CHANNEL_SEARCH_KIND_MOST_STABLE);

        COREWEBVIEW2_RELEASE_CHANNELS releaseChannels = COREWEBVIEW2_RELEASE_CHANNELS_STABLE;
        CHECK_HR(spOptions7->get_ReleaseChannels(&releaseChannels), S_OK);
        CHECK_INT(releaseChannels, COREWEBVIEW2_RELEASE_CHANNELS_NONE);

        CHECK_HR(spOptions7->put_ChannelSearchKind(COREWEBVIEW2_CHANNEL_SEARCH_KIND_LEAST_STABLE), S_OK);
        CHECK_HR(spOptions7->get_ChannelSearchKind(&channelSearchKind), S_OK);
        CHECK_INT(channelSearchKind, COREWEBVIEW2_CHANNEL_SEARCH_KIND_LEAST_STABLE);

        CHECK_HR(spOptions7->put_ReleaseChannels(COREWEBVIEW2_RELEASE_CHANNELS_CANARY), S_OK);
        CHECK_HR(spOptions7->get_ReleaseChannels(&releaseChannels), S_OK);
        CHECK_INT(releaseChannels, COREWEBVIEW2_RELEASE_CHANNELS_CANARY);
    }

    ui::ComPtr<ICoreWebView2EnvironmentOptions8> spOptions8;
    CHECK_HR(spOptions->QueryInterface(IID_ICoreWebView2EnvironmentOptions8, (void**)&spOptions8), S_OK);
    if (spOptions8 != nullptr) {
        COREWEBVIEW2_SCROLLBAR_STYLE scrollBarStyle = COREWEBVIEW2_SCROLLBAR_STYLE_FLUENT_OVERLAY;
        CHECK_HR(spOptions8->get_ScrollBarStyle(&scrollBarStyle), S_OK);
        CHECK_INT(scrollBarStyle, COREWEBVIEW2_SCROLLBAR_STYLE_DEFAULT);

        CHECK_HR(spOptions8->put_ScrollBarStyle(COREWEBVIEW2_SCROLLBAR_STYLE_FLUENT_OVERLAY), S_OK);
        CHECK_HR(spOptions8->get_ScrollBarStyle(&scrollBarStyle), S_OK);
        CHECK_INT(scrollBarStyle, COREWEBVIEW2_SCROLLBAR_STYLE_FLUENT_OVERLAY);
    }

    //未知接口
    IUnknown* pUnknown2 = nullptr;
    CHECK_HR(spOptions->QueryInterface(kIIDUnknown, (void**)&pUnknown2), E_NOINTERFACE);
    CHECK(pUnknown2 == nullptr);
}

/** WebView2Manager 的配置管理 */
void TestWebView2Manager(bool bResourceLoaded)
{
    Section("WebView2Manager");

    ui::WebView2Manager& manager = ui::WebView2Manager::GetInstance();
    CHECK(&manager == &ui::WebView2Manager::GetInstance());

    //默认配置
    CHECK(manager.IsScriptEnabled());
    CHECK(manager.IsWebMessageEnabled());
    CHECK(manager.AreDefaultScriptDialogsEnabled());
    CHECK(manager.AreDefaultContextMenusEnabled());
    CHECK(manager.IsZoomControlEnabled());
    CHECK(manager.AreDevToolsEnabled());
    CHECK(manager.IsEnableF12());
    CHECK(manager.IsEnableF11());
    CHECK(manager.GetUserAgent().empty());
    CHECK(manager.GetAdditionalBrowserArguments().empty());
    CHECK(manager.GetUserDataFolder().empty());
    CHECK(manager.GetBrowserExecutableFolder().empty());

    //配置项读写
    manager.SetScriptEnabled(false);
    CHECK(!manager.IsScriptEnabled());
    manager.SetScriptEnabled(true);

    manager.SetWebMessageEnabled(false);
    CHECK(!manager.IsWebMessageEnabled());
    manager.SetWebMessageEnabled(true);

    manager.SetAreDefaultScriptDialogsEnabled(false);
    CHECK(!manager.AreDefaultScriptDialogsEnabled());
    manager.SetAreDefaultScriptDialogsEnabled(true);

    manager.SetAreDefaultContextMenusEnabled(false);
    CHECK(!manager.AreDefaultContextMenusEnabled());
    manager.SetAreDefaultContextMenusEnabled(true);

    manager.SetZoomControlEnabled(false);
    CHECK(!manager.IsZoomControlEnabled());
    manager.SetZoomControlEnabled(true);

    manager.SetAreDevToolsEnabled(false);
    CHECK(!manager.AreDevToolsEnabled());
    manager.SetAreDevToolsEnabled(true);

    manager.SetEnableF12(false);
    CHECK(!manager.IsEnableF12());
    manager.SetEnableF12(true);

    manager.SetEnableF11(false);
    CHECK(!manager.IsEnableF11());
    manager.SetEnableF11(true);

    manager.SetUserAgent(_T("DuilibTestAgent"));
    CHECK_STR(manager.GetUserAgent(), _T("DuilibTestAgent"));

    manager.SetAdditionalBrowserArguments(_T("--disable-gpu"));
    CHECK_STR(manager.GetAdditionalBrowserArguments(), _T("--disable-gpu"));

    manager.SetUserDataFolder(_T("d:\\webview2_data"));
    CHECK_STR(manager.GetUserDataFolder(), _T("d:\\webview2_data"));

    manager.SetBrowserExecutableFolder(_T("d:\\webview2_runtime"));
    CHECK_STR(manager.GetBrowserExecutableFolder(), _T("d:\\webview2_runtime"));

    //语言：显式设置时优先返回设置值（回归：原先的静态缓存会导致设置值被忽略）
    manager.SetLanguage(_T("zh-CN"));
    CHECK_STR(manager.GetLanguage(), _T("zh-CN"));
    manager.SetLanguage(_T("en-US"));
    CHECK_STR(manager.GetLanguage(), _T("en-US"));

    //语言：未设置时回退到默认语言（由语言资源中的LANGUAGE_NAME决定）
    manager.SetLanguage(_T(""));
    DString expectedLanguage = ui::GlobalManager::GetTextById(DUILIB_LANGUAGE_NAME);
    if (!expectedLanguage.empty()) {
        ui::StringUtil::ReplaceAll(_T("_"), _T("-"), expectedLanguage);
    }
    if (bResourceLoaded) {
        //资源已加载时，默认语言应该能从语言资源中获取到
        CHECK(!expectedLanguage.empty());
    }
    CHECK_STR(manager.GetLanguage(), expectedLanguage);

    //语言：恢复为显式设置后，不再使用缓存的默认语言
    manager.SetLanguage(_T("de-DE"));
    CHECK_STR(manager.GetLanguage(), _T("de-DE"));
    manager.SetLanguage(_T(""));

    //默认的网页缓存目录
    DString userDataFolder = manager.GetDefaultUserDataFolder(_T("test_app"));
    CHECK(!userDataFolder.empty());
    CHECK(userDataFolder.find(_T("webview2_cache")) != DString::npos);
    CHECK(userDataFolder.find(_T("test_app")) != DString::npos);
}

/** WebView2Manager::Initialize 注册的控件创建回调 */
void TestManagerCreateControl()
{
    Section("WebView2Manager::Initialize");

    ui::WebView2Manager& manager = ui::WebView2Manager::GetInstance();
    CHECK(manager.Initialize(_T("d:\\webview2_data"),
                             _T("en-US"),
                             _T("DuilibTestAgent"),
                             _T("--disable-gpu"),
                             _T("d:\\webview2_runtime")));
    CHECK_STR(manager.GetUserDataFolder(), _T("d:\\webview2_data"));
    CHECK_STR(manager.GetLanguage(), _T("en-US"));
    CHECK_STR(manager.GetUserAgent(), _T("DuilibTestAgent"));
    CHECK_STR(manager.GetAdditionalBrowserArguments(), _T("--disable-gpu"));
    CHECK_STR(manager.GetBrowserExecutableFolder(), _T("d:\\webview2_runtime"));

    //注册后，可以通过控件名称创建WebView2控件
    ui::Control* pControl = ui::GlobalManager::Instance().CreateControl(DUI_CTR_WEBVIEW2);
    CHECK(pControl != nullptr);
    if (pControl != nullptr) {
        CHECK_STR(pControl->GetType(), DUI_CTR_WEBVIEW2);
        delete pControl;
        pControl = nullptr;
    }

    //未知的控件名称仍然返回空
    CHECK(ui::GlobalManager::Instance().CreateControl(_T("WebView2NotExists")) == nullptr);
}

/** WebView2Control 未初始化时的行为 */
void TestControlDefaults()
{
    Section("WebView2Control defaults");

    ui::WebView2Control control(nullptr);
    CHECK_STR(control.GetType(), DUI_CTR_WEBVIEW2);
    CHECK(!control.IsInitializing());
    CHECK(!control.IsInitialized());
    CHECK(!control.IsNavigating());
    CHECK_STR(control.GetUrl(), _T(""));
    CHECK_STR(control.GetTitle(), _T(""));
    CHECK_STR(control.GetUserAgent(), _T(""));
    CHECK(!control.CanGoBack());
    CHECK(!control.CanGoForward());
    CHECK(control.GetZoomFactor() == 1.0);
    CHECK_HR(control.GetLastErrorCode(), S_OK);

    //默认的WebView2配置（回归：这些成员必须初始化，不能是未初始化的值）
    CHECK(control.IsScriptEnabled());
    CHECK(control.IsWebMessageEnabled());
    CHECK(control.AreDefaultScriptDialogsEnabled());
    CHECK(control.AreDefaultContextMenusEnabled());
    CHECK(control.IsZoomControlEnabled());
    CHECK(control.AreDevToolsEnabled());
    CHECK(control.IsEnableF12());
    CHECK(control.IsEnableF11());

    //未初始化时，接口对象为空
    CHECK(control.GetWebView2Environment() == nullptr);
    CHECK(control.GetWebView2Controller() == nullptr);
    CHECK(control.GetWebView2() == nullptr);

    //未初始化时，依赖初始化结果的接口返回失败，不会崩溃
    CHECK(!control.NavigateBack());
    CHECK(!control.NavigateForward());
    CHECK(!control.Refresh());
    CHECK(!control.Stop());
    CHECK(!control.CapturePreview(_T("test_capture.png")));
    CHECK(!control.OpenDevToolsWindow());
    CHECK(control.MakeImageSnapshot() == nullptr);

    //配置项的设置（未初始化时只记录设置值）
    CHECK(control.SetScriptEnabled(false));
    CHECK(!control.IsScriptEnabled());
    CHECK(control.SetWebMessageEnabled(false));
    CHECK(!control.IsWebMessageEnabled());
    CHECK(control.SetAreDefaultScriptDialogsEnabled(false));
    CHECK(!control.AreDefaultScriptDialogsEnabled());
    CHECK(control.SetAreDefaultContextMenusEnabled(false));
    CHECK(!control.AreDefaultContextMenusEnabled());
    CHECK(control.SetZoomControlEnabled(false));
    CHECK(!control.IsZoomControlEnabled());
    control.SetAreDevToolsEnabled(false);
    CHECK(!control.AreDevToolsEnabled());
    control.SetEnableF12(false);
    CHECK(!control.IsEnableF12());
    control.SetEnableF11(false);
    CHECK(!control.IsEnableF11());

    //UserAgent：未初始化时返回本地设置的值
    CHECK(control.SetUserAgent(_T("DuilibTestAgent")));
    CHECK_STR(control.GetUserAgent(), _T("DuilibTestAgent"));

    //缩放因子：未初始化时设置失败
    CHECK(!control.SetZoomFactor(2.0));
    CHECK(control.GetZoomFactor() == 1.0);
    CHECK(control.GetLastErrorCode() != S_OK);
}

/** WebView2Control 初始URL的拼接规则 */
void TestControlInitUrl(bool bResourceLoaded)
{
    Section("WebView2Control init url");

    ui::WebView2Control control(nullptr);
    CHECK_STR(control.GetInitURL(), _T(""));

    //普通URL：不做任何转换
    control.SetInitURL(_T("https://www.example.com/a.html?x=1"));
    CHECK_STR(control.GetInitURL(), _T("https://www.example.com/a.html?x=1"));

    //本地文件标志为true，但URL自带协议前缀：不做转换
    control.SetInitUrlIsLocalFile(true);
    CHECK_STR(control.GetInitURL(), _T("https://www.example.com/a.html?x=1"));

    control.SetInitURL(_T("file:///C:/temp/a.html"));
    CHECK_STR(control.GetInitURL(), _T("file:///C:/temp/a.html"));

    //非本地文件模式：只把反斜杠替换为正斜杠
    control.SetInitUrlIsLocalFile(false);
    control.SetInitURL(_T("C:\\temp\\a.html"));
    CHECK_STR(control.GetInitURL(), _T("C:/temp/a.html"));

    //本地文件模式 + 存在的绝对路径：转换为file协议URL
    std::filesystem::path tempFile =
        std::filesystem::temp_directory_path() / "duilib_webview2_init_url_test.html";
    {
        std::ofstream outFile(tempFile, std::ios::binary);
        outFile << "webview2 init url test";
    }
    if (std::filesystem::exists(tempFile)) {
        control.SetInitUrlIsLocalFile(true);
        control.SetInitURL(tempFile.wstring());

        ui::FilePath expectedPath(tempFile.wstring());
        expectedPath.NormalizeFilePath();
        DString expectedUrl = _T("file:///") + expectedPath.ToString();
        ui::StringUtil::ReplaceAll(_T("\\"), _T("/"), expectedUrl);
        CHECK_STR(control.GetInitURL(), expectedUrl);

        std::error_code ec;
        std::filesystem::remove(tempFile, ec);
    }

    //本地文件模式 + 不存在的相对路径：回退为file协议URL（需要资源路径，资源加载后执行）
    if (bResourceLoaded) {
        control.SetInitUrlIsLocalFile(true);
        control.SetInitURL(_T("duilib_webview2_not_exists_file.html"));
        CHECK_STR(control.GetInitURL(), _T("file:///duilib_webview2_not_exists_file.html"));
    }
}

} //namespace

int main()
{
#if defined(_WIN32) && defined(_DEBUG)
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
#endif

    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);

    //关闭动画，避免启动资源时的额外线程/定时器影响测试
    ui::GlobalManager::Instance().SetAnimationEnabled(false);
    const bool bStartup = ui::GlobalManager::Instance().Startup(
        ui::LocalFilesResParam(ui::FilePath(DString(_T(DUILIB_TEST_RESOURCE_DIR)))));

    TestComPtr();
    TestComCallback();
    TestEnvironmentOptions();
    TestWebView2Manager(bStartup);
    TestManagerCreateControl();
    TestControlDefaults();
    TestControlInitUrl(bStartup);

    ui::GlobalManager::Instance().Shutdown();

    if (g_failedCount == 0) {
        std::cerr << "All checks passed: " << g_checkCount << std::endl;
        return 0;
    }
    std::cerr << "Failed checks: " << g_failedCount << " / " << g_checkCount << std::endl;
    return (g_failedCount > 0) ? 1 : 0;
}

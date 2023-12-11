#pragma once

#include "popover.h"

namespace nim_comp {

class AlertFooter :public PopoverFooter
{
public:
    AlertFooter(const std::wstring& strOk, const std::wstring& strCancel);
    ~AlertFooter() = default;

    std::wstring GetType() const override;
    virtual void OnInit() override;
};

}
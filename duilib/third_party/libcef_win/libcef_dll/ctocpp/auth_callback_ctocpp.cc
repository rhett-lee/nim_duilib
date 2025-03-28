// Copyright (c) 2025 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// ---------------------------------------------------------------------------
//
// This file was generated by the CEF translator tool. If making changes by
// hand only do so within the body of existing method and function
// implementations. See the translator.README.txt file in the tools directory
// for more information.
//
// $hash=5499f31d219589c45a08eee0da0def32175c16a7$
//

#include "libcef_dll/ctocpp/auth_callback_ctocpp.h"

#include "libcef_dll/shutdown_checker.h"

// VIRTUAL METHODS - Body may be edited by hand.

NO_SANITIZE("cfi-icall")
void CefAuthCallbackCToCpp::Continue(const CefString& username,
                                     const CefString& password) {
  shutdown_checker::AssertNotShutdown();

  auto* _struct = GetStruct();
  if (!_struct->cont) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Unverified params: username, password

  // Execute
  _struct->cont(_struct, username.GetStruct(), password.GetStruct());
}

NO_SANITIZE("cfi-icall") void CefAuthCallbackCToCpp::Cancel() {
  shutdown_checker::AssertNotShutdown();

  auto* _struct = GetStruct();
  if (!_struct->cancel) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->cancel(_struct);
}

// CONSTRUCTOR - Do not edit by hand.

CefAuthCallbackCToCpp::CefAuthCallbackCToCpp() {}

// DESTRUCTOR - Do not edit by hand.

CefAuthCallbackCToCpp::~CefAuthCallbackCToCpp() {
  shutdown_checker::AssertNotShutdown();
}

template <>
cef_auth_callback_t*
CefCToCppRefCounted<CefAuthCallbackCToCpp,
                    CefAuthCallback,
                    cef_auth_callback_t>::UnwrapDerived(CefWrapperType type,
                                                        CefAuthCallback* c) {
  CHECK(false) << __func__ << " called with unexpected class type " << type;
  return nullptr;
}

template <>
CefWrapperType CefCToCppRefCounted<CefAuthCallbackCToCpp,
                                   CefAuthCallback,
                                   cef_auth_callback_t>::kWrapperType =
    WT_AUTH_CALLBACK;

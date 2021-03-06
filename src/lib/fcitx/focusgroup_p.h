/*
 * Copyright (C) 2016~2016 by CSSlayer
 * wengxt@gmail.com
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the file COPYING. If not,
 * see <http://www.gnu.org/licenses/>.
 */
#ifndef _FCITX_FOCUSGROUP_P_H_
#define _FCITX_FOCUSGROUP_P_H_

#include "fcitx-utils/intrusivelist.h"
#include "focusgroup.h"
#include <unordered_set>

namespace fcitx {

class InputContextManager;

class FocusGroupPrivate {
public:
    FocusGroupPrivate(FocusGroup *q, InputContextManager &manager_) : q_ptr(q), manager(manager_), focus(nullptr) {}

    FocusGroup *q_ptr;
    InputContextManager &manager;
    InputContext *focus;
    std::unordered_set<InputContext *> ics;

    IntrusiveListNode listNode;
    FCITX_DECLARE_PUBLIC(FocusGroup);
};
}

#endif // _FCITX_FOCUSGROUP_P_H_

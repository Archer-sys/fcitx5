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
#ifndef _FCITX_TEXT_H_
#define _FCITX_TEXT_H_

#include "fcitxcore_export.h"
#include <fcitx-utils/flags.h>
#include <fcitx-utils/macros.h>
#include <memory>
#include <string>

namespace fcitx {

enum class TextFormatFlag : int {
    UnderLine = (1 << 0), /**< underline is a flag */
    HighLight = (1 << 1), /**< highlight the preedit */
    Bold = (1 << 2),
    Strike = (1 << 3),
    None = 0,
};

typedef Flags<TextFormatFlag> TextFormatFlags;
class TextPrivate;
class FCITXCORE_EXPORT Text {
public:
    Text();
    virtual ~Text();
    Text(const Text &other);
    Text(Text &&other);

    Text &operator=(Text text) {
        using std::swap;
        swap(*this, text);
        return *this;
    }

    int cursor() const;
    void setCursor(int pos = -1);
    void clear();
    void append(const std::string &str, TextFormatFlags flag = TextFormatFlag::None);
    const std::string &stringAt(int idx) const;
    TextFormatFlags formatAt(int idx) const;
    size_t size() const;
    std::string toString() const;

private:
    std::unique_ptr<TextPrivate> d_ptr;
    FCITX_DECLARE_PRIVATE(Text);
};
}

#endif // _FCITX_TEXT_H_

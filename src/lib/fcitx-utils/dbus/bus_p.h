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
#ifndef _FCITX_UTILS_DBUS_BUS_P_H_
#define _FCITX_UTILS_DBUS_BUS_P_H_

#include "dbus/bus.h"
#include "sd-bus-wrap.h"

namespace fcitx {

namespace dbus {

int SDMessageCallback(sd_bus_message *m, void *userdata, sd_bus_error *);

class ScopedSDBusError {
public:
    ScopedSDBusError() { m_error = SD_BUS_ERROR_NULL; }
    ~ScopedSDBusError() { sd_bus_error_free(&m_error); }

    sd_bus_error &error() { return m_error; }

private:
    sd_bus_error m_error;
};

class SDVTableSlot : public Slot {
public:
    SDVTableSlot(std::vector<sd_bus_vtable> vtable_, Bus *bus_, const std::string &path_, const std::string &interface_)
        : slot(nullptr), vtable(vtable_), bus(bus_), path(path_), interface(interface_) {}

    ~SDVTableSlot() { sd_bus_slot_unref(slot); }

    sd_bus_slot *slot;
    std::vector<sd_bus_vtable> vtable;
    Bus *bus;
    std::string path;
    std::string interface;
};

class SDSlot : public Slot {
public:
    SDSlot(MessageCallback callback_) : callback(callback_), slot(nullptr) {}

    ~SDSlot() { sd_bus_slot_unref(slot); }

    MessageCallback callback;
    sd_bus_slot *slot;
};

class SDSubTreeSlot : public SDSlot {
public:
    SDSubTreeSlot(MessageCallback callback_, EnumerateObjectCallback enumerator_)
        : SDSlot(callback_), enumerator(enumerator_), enumSlot(nullptr) {}

    ~SDSubTreeSlot() { sd_bus_slot_unref(enumSlot); }

    EnumerateObjectCallback enumerator;
    sd_bus_slot *enumSlot;
};
}
}

#endif // _FCITX_UTILS_DBUS_BUS_P_H_

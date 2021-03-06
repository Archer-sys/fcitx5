/*
 * Copyright (C) 2015~2015 by CSSlayer
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

#include <exception>
#include <functional>
#include <iostream>

#if defined(__COVERITY__) && !defined(__INCLUDE_LEVEL__)
#define __INCLUDE_LEVEL__ 2
#endif
#include "event.h"
#include <systemd/sd-event.h>

namespace fcitx {

static uint32_t IOEventFlagsToEpollFlags(IOEventFlags flags) {
    uint32_t result = 0;
    if (flags & IOEventFlag::In) {
        result |= EPOLLIN;
    }
    if (flags & IOEventFlag::Out) {
        result |= EPOLLOUT;
    }
    if (flags & IOEventFlag::Err) {
        result |= EPOLLERR;
    }
    if (flags & IOEventFlag::Hup) {
        result |= EPOLLHUP;
    }
    if (flags & IOEventFlag::EdgeTrigger) {
        result |= EPOLLET;
    }
    return result;
}

static IOEventFlags EpollFlagsToIOEventFlags(uint32_t flags) {
    return ((flags & EPOLLIN) ? IOEventFlag::In : IOEventFlags()) |
           ((flags & EPOLLOUT) ? IOEventFlag::Out : IOEventFlags()) |
           ((flags & EPOLLERR) ? IOEventFlag::Err : IOEventFlags()) |
           ((flags & EPOLLHUP) ? IOEventFlag::Hup : IOEventFlags()) |
           ((flags & EPOLLET) ? IOEventFlag::EdgeTrigger : IOEventFlags());
}

template <typename Interface>
struct SDEventSource : public Interface {
public:
    ~SDEventSource() {
        if (m_eventSource) {
            sd_event_source_unref(m_eventSource);
        }
    }

    void setEventSource(sd_event_source *event) { m_eventSource = event; }

    virtual bool isEnabled() const override {
        int result = 0, err;
        if ((err = sd_event_source_get_enabled(m_eventSource, &result)) < 0) {
            throw EventLoopException(err);
        }
        return result != 0;
    }

    virtual void setEnabled(bool enabled) override {
        sd_event_source_set_enabled(m_eventSource, enabled ? SD_EVENT_ON : SD_EVENT_OFF);
    }

    virtual bool isOneShot() const override {
        int result = 0, err;
        if ((err = sd_event_source_get_enabled(m_eventSource, &result)) < 0) {
            throw EventLoopException(err);
        }
        return result == SD_EVENT_ONESHOT;
    }

    virtual void setOneShot() override { sd_event_source_set_enabled(m_eventSource, SD_EVENT_ONESHOT); }

protected:
    sd_event_source *m_eventSource;
};

struct SDEventSourceIO : public SDEventSource<EventSourceIO> {
    SDEventSourceIO(IOCallback _callback) : SDEventSource(), callback(_callback) {}

    virtual int fd() const override {
        int ret = sd_event_source_get_io_fd(m_eventSource);
        if (ret < 0) {
            throw EventLoopException(ret);
        }
        return ret;
    }

    virtual void setFd(int fd) override {
        int ret = sd_event_source_set_io_fd(m_eventSource, fd);
        if (ret < 0) {
            throw EventLoopException(ret);
        }
    }

    virtual IOEventFlags events() const override {
        uint32_t events;
        int ret = sd_event_source_get_io_events(m_eventSource, &events);
        if (ret < 0) {
            throw EventLoopException(ret);
        }
        return EpollFlagsToIOEventFlags(events);
    }

    virtual void setEvents(IOEventFlags flags) override {
        int ret = sd_event_source_set_io_events(m_eventSource, IOEventFlagsToEpollFlags(flags));
        if (ret < 0) {
            throw EventLoopException(ret);
        }
    }

    virtual IOEventFlags revents() const override {
        uint32_t revents;
        int ret = sd_event_source_get_io_revents(m_eventSource, &revents);
        if (ret < 0) {
            throw EventLoopException(ret);
        }
        return EpollFlagsToIOEventFlags(revents);
    }

    IOCallback callback;
};

struct SDEventSourceTime : public SDEventSource<EventSourceTime> {
    SDEventSourceTime(TimeCallback _callback) : SDEventSource(), callback(_callback) {}

    virtual uint64_t time() const override {
        uint64_t time;
        int err = sd_event_source_get_time(m_eventSource, &time);
        if (err < 0) {
            throw EventLoopException(err);
        }
        return time;
    }

    virtual void setTime(uint64_t time) override {
        int ret = sd_event_source_set_time(m_eventSource, time);
        if (ret < 0) {
            throw EventLoopException(ret);
        }
    }

    virtual uint64_t accuracy() const override {
        uint64_t time;
        int err = sd_event_source_get_time_accuracy(m_eventSource, &time);
        if (err < 0) {
            throw EventLoopException(err);
        }
        return time;
    }

    virtual void setAccuracy(uint64_t time) override {
        int ret = sd_event_source_set_time_accuracy(m_eventSource, time);
        if (ret < 0) {
            throw EventLoopException(ret);
        }
    }

    virtual clockid_t clock() const override {
        clockid_t clock;
        int err = sd_event_source_get_time_clock(m_eventSource, &clock);
        if (err < 0) {
            throw EventLoopException(err);
        }
        return clock;
    }

    TimeCallback callback;
};

struct EventLoopPrivate {
    EventLoopPrivate() {
        if (sd_event_new(&event) < 0) {
            throw std::runtime_error("Create sd_event failed.");
        }
    }

    ~EventLoopPrivate() { sd_event_unref(event); }

    sd_event *event;
};

EventLoop::EventLoop() : d_ptr(std::make_unique<EventLoopPrivate>()) {}

EventLoop::~EventLoop() {}

const char *EventLoop::impl() { return "sd-event"; }

void *EventLoop::nativeHandle() {
    FCITX_D();
    return d->event;
}

bool EventLoop::exec() {
    FCITX_D();
    int r = sd_event_loop(d->event);
    return r >= 0;
}

void EventLoop::quit() {
    FCITX_D();
    sd_event_exit(d->event, 0);
}

int IOEventCallback(sd_event_source *, int fd, uint32_t revents, void *userdata) {
    auto source = static_cast<SDEventSourceIO *>(userdata);
    try {
        auto result = source->callback(source, fd, EpollFlagsToIOEventFlags(revents));
        return result ? 0 : -1;
    } catch (...) {
        // some abnormal things threw
        abort();
    }
    return -1;
}

EventSourceIO *EventLoop::addIOEvent(int fd, IOEventFlags flags, IOCallback callback) {
    FCITX_D();
    auto source = std::make_unique<SDEventSourceIO>(callback);
    sd_event_source *sdEventSource;
    int err;
    if ((err = sd_event_add_io(d->event, &sdEventSource, fd, IOEventFlagsToEpollFlags(flags), IOEventCallback,
                               source.get())) < 0) {
        throw EventLoopException(err);
    }
    source->setEventSource(sdEventSource);
    return source.release();
}

int TimeEventCallback(sd_event_source *, uint64_t usec, void *userdata) {
    auto source = static_cast<SDEventSourceTime *>(userdata);

    try {
        auto result = source->callback(source, usec);
        return result ? 0 : -1;
    } catch (...) {
        // some abnormal things threw
        abort();
    }
    return -1;
}

EventSourceTime *EventLoop::addTimeEvent(clockid_t clock, uint64_t usec, uint64_t accuracy, TimeCallback callback) {
    FCITX_D();
    auto source = std::make_unique<SDEventSourceTime>(callback);
    sd_event_source *sdEventSource;
    int err;
    if ((err = sd_event_add_time(d->event, &sdEventSource, clock, usec, accuracy, TimeEventCallback, source.get())) <
        0) {
        throw EventLoopException(err);
    }
    source->setEventSource(sdEventSource);
    return source.release();
}
}

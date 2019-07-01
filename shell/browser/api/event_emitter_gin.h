// Copyright (c) 2019 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

// TODO(deermichel): remove _gin suffix after removing mate

#ifndef SHELL_BROWSER_API_EVENT_EMITTER_GIN_H_
#define SHELL_BROWSER_API_EVENT_EMITTER_GIN_H_

#include <utility>
#include <vector>

#include "base/optional.h"
#include "content/public/browser/browser_thread.h"
#include "electron/shell/common/api/api.mojom.h"
#include "gin/wrappable.h"
#include "shell/common/api/event_emitter_caller.h"

namespace content {
class RenderFrameHost;
}

namespace gin {

namespace internal {

v8::Local<v8::Object> CreateJSEvent(
    v8::Isolate* isolate,
    v8::Local<v8::Object> object,
    content::RenderFrameHost* sender,
    base::Optional<electron::mojom::ElectronBrowser::MessageSyncCallback>
        callback);
v8::Local<v8::Object> CreateCustomEvent(v8::Isolate* isolate,
                                        v8::Local<v8::Object> object,
                                        v8::Local<v8::Object> event);
v8::Local<v8::Object> CreateEventFromFlags(v8::Isolate* isolate, int flags);

}  // namespace internal

// Provide helperers to emit event in JavaScript.
template <typename T>
class EventEmitter : public Wrappable<T> {
 public:
  typedef std::vector<v8::Local<v8::Value>> ValueArray;

  // Make the convinient methods visible:
  // https://isocpp.org/wiki/faq/templates#nondependent-name-lookup-members
  v8::Isolate* isolate() const { return Wrappable<T>::isolate(); }
  v8::Local<v8::Object> GetWrapper() const {
    return Wrappable<T>::GetWrapper();
  }
  v8::MaybeLocal<v8::Object> GetWrapper(v8::Isolate* isolate) {
    return Wrappable<T>::GetWrapper(isolate);
  }

  // this.emit(name, event, args...);
  template <typename... Args>
  bool EmitCustomEvent(const base::StringPiece& name,
                       v8::Local<v8::Object> event,
                       Args&&... args) {
    return EmitWithEvent(
        name, internal::CreateCustomEvent(isolate(), GetWrapper(), event),
        std::forward<Args>(args)...);
  }

  // this.emit(name, new Event(flags), args...);
  template <typename... Args>
  bool EmitWithFlags(const base::StringPiece& name, int flags, Args&&... args) {
    return EmitCustomEvent(name,
                           internal::CreateEventFromFlags(isolate(), flags),
                           std::forward<Args>(args)...);
  }

  // this.emit(name, new Event(), args...);
  template <typename... Args>
  bool Emit(const base::StringPiece& name, Args&&... args) {
    return EmitWithSender(name, nullptr, base::nullopt,
                          std::forward<Args>(args)...);
  }

  // this.emit(name, new Event(sender, message), args...);
  template <typename... Args>
  bool EmitWithSender(
      const base::StringPiece& name,
      content::RenderFrameHost* sender,
      base::Optional<electron::mojom::ElectronBrowser::MessageSyncCallback>
          callback,
      Args&&... args) {
    v8::Locker locker(isolate());
    v8::HandleScope handle_scope(isolate());
    v8::Local<v8::Object> wrapper = GetWrapper();
    if (wrapper.IsEmpty()) {
      return false;
    }
    v8::Local<v8::Object> event = internal::CreateJSEvent(
        isolate(), wrapper, sender, std::move(callback));
    return EmitWithEvent(name, event, std::forward<Args>(args)...);
  }

 protected:
  EventEmitter() {}

 private:
  // this.emit(name, event, args...);
  template <typename... Args>
  bool EmitWithEvent(const base::StringPiece& name,
                     v8::Local<v8::Object> event,
                     Args&&... args) {
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
    v8::Locker locker(isolate());
    v8::HandleScope handle_scope(isolate());
    EmitEvent(isolate(), GetWrapper(), name, event,
              std::forward<Args>(args)...);
    auto context = isolate()->GetCurrentContext();
    v8::Local<v8::Value> defaultPrevented;
    if (event->Get(context, StringToV8(isolate(), "defaultPrevented"))
            .ToLocal(&defaultPrevented)) {
      return defaultPrevented->BooleanValue(isolate());
    }
    return false;
  }

  DISALLOW_COPY_AND_ASSIGN(EventEmitter);
};

}  // namespace gin

#endif  // SHELL_BROWSER_API_EVENT_EMITTER_GIN_H_

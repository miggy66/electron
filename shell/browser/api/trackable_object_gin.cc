// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

// TODO(deermichel): remove _gin suffix after removing mate

#include "shell/browser/api/trackable_object_gin.h"

#include <memory>

#include "base/bind.h"
#include "base/supports_user_data.h"
#include "shell/browser/atom_browser_main_parts.h"

namespace gin {

namespace {

const char* kTrackedObjectKey = "TrackedObjectKey";

class IDUserData : public base::SupportsUserData::Data {
 public:
  explicit IDUserData(int32_t id) : id_(id) {}

  operator int32_t() const { return id_; }

 private:
  int32_t id_;

  DISALLOW_COPY_AND_ASSIGN(IDUserData);
};

}  // namespace

TrackableObjectBase::TrackableObjectBase() : weak_factory_(this) {
  electron::AtomBrowserMainParts::Get()->RegisterDestructionCallback(
      GetDestroyClosure());
}

TrackableObjectBase::~TrackableObjectBase() {}

base::OnceClosure TrackableObjectBase::GetDestroyClosure() {
  return base::BindOnce(&TrackableObjectBase::Destroy,
                        weak_factory_.GetWeakPtr());
}

void TrackableObjectBase::Destroy() {
  delete this;
}

void TrackableObjectBase::AttachAsUserData(base::SupportsUserData* wrapped) {
  wrapped->SetUserData(kTrackedObjectKey,
                       std::make_unique<IDUserData>(weak_map_id_));
}

// static
int32_t TrackableObjectBase::GetIDFromWrappedClass(
    base::SupportsUserData* wrapped) {
  if (wrapped) {
    auto* id =
        static_cast<IDUserData*>(wrapped->GetUserData(kTrackedObjectKey));
    if (id)
      return *id;
  }
  return 0;
}

}  // namespace gin

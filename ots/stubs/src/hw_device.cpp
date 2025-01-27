#include "device/device.hpp"

namespace hw {
    device_registry::device_registry() { }

    device_registry::~device_registry() { }

    std::unique_ptr<device> device_registry::get_device(const std::string& device_descriptor) {
        return nullptr;
    }
}

#pragma once

#include <memory>

namespace infinite_fire {

struct HookImpl;

class InfiniteFireMod {

public:
    static InfiniteFireMod& getInstance();

    InfiniteFireMod() = default;

    bool load();
    bool enable();
    bool disable();

private:
    std::unique_ptr<HookImpl> mHookImpl;
};

} // namespace infinite_fire
#pragma once

#include "mc/world/level/BlockPos.h"
#include <ll/api/mod/NativeMod.h>
#include <memory>
#include <unordered_set>
#include <optional>

namespace infinite_fire {

struct Impl;

extern std::unordered_set<BlockPos> gFirePositions;

struct Config {
    int  accelerationFactor = 20;   // 燃烧加速倍率，数值越大烧得越快
    int  accelerationTick   = 1;    // 任务执行频率（游戏刻），数值越小越频繁
    bool keepFireBurning    = true; // 火焰是否无法被扑灭
    int  version            = 2;
};

class InfiniteFireMod {
public:
    static InfiniteFireMod& getInstance();

    InfiniteFireMod() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    bool load();
    bool enable();
    bool disable();

    Config& getConfig();
    void    saveConfig();

private:
    ll::mod::NativeMod&   mSelf;
    std::unique_ptr<Impl> mImpl;
    std::optional<Config> mConfig;
};

} // namespace infinite_fire
#include "MyMod.h"

#include "ll/api/memory/Hook.h"
#include "ll/api/mod/RegisterHelper.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/FireBlock.h"
#include "mc/world/level/Level.h" 

namespace infinite_fire {

// 阻止火焰被扑灭
LL_TYPE_INSTANCE_HOOK(
    LevelExtinguishFireHook,
    ll::memory::HookPriority::Highest,
    Level,
    &Level::$extinguishFire,
    bool,
    [[maybe_unused]] BlockSource&     region,
    [[maybe_unused]] BlockPos const&  pos,
    [[maybe_unused]] unsigned char    face,
    [[maybe_unused]] Actor* source
) {
    return false;
}

// 加速火焰的更新和蔓延
LL_TYPE_INSTANCE_HOOK(
    FireBlockNeighborChangedHook,
    ll::memory::HookPriority::Normal,
    FireBlock,
    &FireBlock::$neighborChanged,
    void,
    BlockSource&     region,
    BlockPos const&  pos,
    BlockPos const&  neighborPos
) {
    origin(region, pos, neighborPos);

    auto const& block = region.getBlock(pos);
    if (block.isAir()) {
        return;
    }
    region.addToTickingQueue(pos, block, 1, 0, false);
}

struct HookImpl {
    ll::memory::HookRegistrar<
        LevelExtinguishFireHook,
        FireBlockNeighborChangedHook
    > mHookRegistrar;
};

InfiniteFireMod& InfiniteFireMod::getInstance() {
    static InfiniteFireMod instance;
    return instance;
}

bool InfiniteFireMod::load() {
    return true; 
}

bool InfiniteFireMod::enable() {
    mHookImpl = std::make_unique<HookImpl>();
    return true;
}

bool InfiniteFireMod::disable() {
    mHookImpl.reset();
    return true;
}

} // namespace infinite_fire

LL_REGISTER_MOD(infinite_fire::InfiniteFireMod, infinite_fire::InfiniteFireMod::getInstance());
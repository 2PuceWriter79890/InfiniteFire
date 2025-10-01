#include "MyMod.h"

#include "ll/api/memory/Hook.h"
#include "ll/api/mod/RegisterHelper.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/FireBlock.h"

namespace infinite_fire {

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

    constexpr int accelerationFactor = 5;
    for (int i = 0; i < accelerationFactor; ++i) {
        // void addToTickingQueue(BlockPos const&, Block const&, int tickDelay, int priorityOffset, bool skipOverrides);
        region.addToTickingQueue(pos, block, 1, 0, false);
    }
}

LL_TYPE_INSTANCE_HOOK(
    FireBlockMayPlaceHook,
    ll::memory::HookPriority::Highest,
    FireBlock,
    &FireBlock::$mayPlace,
    bool,
    [[maybe_unused]] BlockSource& region,
    [[maybe_unused]] BlockPos const& pos
) {
    return true;
}

struct HookImpl {
    ll::memory::HookRegistrar<
        FireBlockNeighborChangedHook,
        FireBlockMayPlaceHook
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
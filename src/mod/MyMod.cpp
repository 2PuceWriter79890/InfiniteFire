#include "MyMod.h"

#include "ll/api/Config.h"
#include "ll/api/io/Logger.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/world/BlockChangedEvent.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/mod/RegisterHelper.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/thread/ServerThreadExecutor.h"
#include "ll/api/utils/ErrorUtils.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/BlockPos.h"

using namespace ll::literals::chrono_literals;

namespace infinite_fire {

std::unordered_set<BlockPos> gFirePositions;

LL_TYPE_INSTANCE_HOOK(
    LevelExtinguishFireHook,
    ll::memory::HookPriority::Highest,
    Level,
    &Level::$extinguishFire,
    bool,
    [[maybe_unused]] BlockSource&,
    [[maybe_unused]] BlockPos const&,
    [[maybe_unused]] unsigned char,
    [[maybe_unused]] Actor*) {
    return !InfiniteFireMod::getInstance().getConfig().keepFireBurning;
}

struct Impl {
    ll::memory::HookRegistrar<LevelExtinguishFireHook> mHookRegistrar;

    ll::event::ListenerPtr mBlockChangeListener;
    std::shared_ptr<bool>  mIsTaskRunning = std::make_shared<bool>(false);

    ll::coro::CoroTask<> startUpdateTask() const {
        *mIsTaskRunning = true;
        while (*mIsTaskRunning) {
            auto& config = InfiniteFireMod::getInstance().getConfig();

            co_await ll::chrono::ticks(config.accelerationTick);

            auto level = ll::service::getLevel();
            if (!level) continue;

            auto dimension = level->getDimension(0).lock();
            if (!dimension) continue;

            auto& region = const_cast<BlockSource&>(dimension->getBlockSourceFromMainChunkSource());
            auto& random = level->getThreadRandom();

            auto positions_copy = gFirePositions;
            for (const auto& pos : positions_copy) {
                auto const& block = region.getBlock(pos);
                if (block.getTypeName() == "minecraft:fire") {
                    for (int i = 0; i < config.accelerationFactor; ++i) {
                        block.queuedTick(region, pos, random);
                    }
                }
            }
        }
        co_return;
    }

    Impl() {
        mBlockChangeListener = ll::event::EventBus::getInstance().emplaceListener<ll::event::BlockChangedEvent>(
            [](ll::event::BlockChangedEvent& event) {
                if (event.newBlock().getTypeName() == "minecraft:fire") {
                    gFirePositions.insert(event.pos());
                } else if (event.previousBlock().getTypeName() == "minecraft:fire") {
                    gFirePositions.erase(event.pos());
                }
            });

        startUpdateTask().launch(ll::thread::ServerThreadExecutor::getDefault());
    }

    ~Impl() {
        ll::event::EventBus::getInstance().removeListener(mBlockChangeListener);
        *mIsTaskRunning = false;
    }
};


InfiniteFireMod& InfiniteFireMod::getInstance() {
    static InfiniteFireMod instance;
    return instance;
}

bool InfiniteFireMod::load() {
    mConfig.emplace();
    try {
        ll::config::loadConfig(*mConfig, getSelf().getConfigDir() / "config.json");
    } catch (...) {
        ll::error_utils::printCurrentException(getSelf().getLogger());
    }
    saveConfig();
    getSelf().getLogger().info("InfiniteFire loaded!");
    return true;
}

bool InfiniteFireMod::enable() {
    mImpl = std::make_unique<Impl>();
    getSelf().getLogger().info("InfiniteFire enabled!");
    return true;
}

bool InfiniteFireMod::disable() {
    gFirePositions.clear();
    mImpl.reset();
    getSelf().getLogger().info("InfiniteFire disabled!");
    return true;
}

Config& InfiniteFireMod::getConfig() { return mConfig.value(); }

void InfiniteFireMod::saveConfig() { ll::config::saveConfig(*mConfig, getSelf().getConfigDir() / "config.json"); }

} // namespace infinite_fire

LL_REGISTER_MOD(infinite_fire::InfiniteFireMod, infinite_fire::InfiniteFireMod::getInstance());
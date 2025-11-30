#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <random>
using namespace geode::prelude;

std::random_device rd;
std::mt19937 gen(rd());

class $modify(LazyPlayerObject, PlayerObject) {
    static inline int frameCounter = 0;
    static inline bool isLazy = false;
    static inline int lazyTimer = 0;
    static inline bool isDragging = false;
    static inline int dragTimer = 0;
    static inline bool movingBackwards = false;
    static inline int backwardsTimer = 0;
    static inline bool isSlowedDown = false;
    static inline int slowTimer = 0;
    static inline bool hasWeakJump = false;
    static inline int weakJumpTimer = 0;
    static inline bool isStuttering = false;
    static inline int stutterTimer = 0;
    static inline int stutterState = 0;
    static inline bool isSleeping = false;
    static inline int sleepTimer = 0;
    static inline bool isFalling = false;
    static inline int fallTimer = 0;
    static inline bool isHeavy = false;
    static inline int heavyTimer = 0;
    static inline bool isDistracted = false;
    static inline int distractTimer = 0;
    static inline int distractCounter = 0;

    bool isModEnabled() {
        return Mod::get()->getSettingValue<bool>("enabled");
    }

    int getEventFrequency() {
        return Mod::get()->getSettingValue<int64_t>("event-frequency");
    }

    int getEventChance() {
        return Mod::get()->getSettingValue<int64_t>("event-chance");
    }

    int getEventWeight(const std::string& eventName) {
        return Mod::get()->getSettingValue<int64_t>(eventName + "-weight");
    }

    int selectRandomEvent() {
        int weights[9] = {
            getEventWeight("faint"),
            getEventWeight("backwards"),
            getEventWeight("tired"),
            getEventWeight("weak-jump"),
            getEventWeight("stutter"),
            getEventWeight("sleep"),
            getEventWeight("stumble"),
            getEventWeight("heavy"),
            getEventWeight("distracted")
        };

        int totalWeight = 0;
        for (int i = 0; i < 9; i++) {
            totalWeight += weights[i];
        }

        if (totalWeight == 0) return -1;

        std::uniform_int_distribution<> dist(1, totalWeight);
        int randomValue = dist(gen);
        
        int cumulative = 0;
        for (int i = 0; i < 9; i++) {
            cumulative += weights[i];
            if (randomValue <= cumulative) {
                return i + 1; 
            }
        }

        return 1;
    }

    void update(float dt) {
        PlayerObject::update(dt);

        if (!isModEnabled()) return;

        frameCounter++;
        int minFrames = getEventFrequency();
        int chanceValue = getEventChance();

        if (frameCounter > minFrames && !isLazy && !movingBackwards && !isSlowedDown && !isStuttering && !isSleeping && !isFalling && !hasWeakJump && !isHeavy && !isDistracted) {
            std::uniform_int_distribution<> lazyChance(1, 200);
            if (lazyChance(gen) <= chanceValue) {
                int event = selectRandomEvent();

                if (event == -1) {
                    frameCounter = 0;
                    return;
                }

                if (event == 2) {
                    isLazy = true;
                    std::uniform_int_distribution<> duration(60, 120);
                    lazyTimer = duration(gen);
                    log::info("Player fainted - all controls blocked!");
                }
                else if (event == 3) {
                    movingBackwards = true;
                    std::uniform_int_distribution<> duration(30, 60);
                    backwardsTimer = duration(gen);
                    log::info("Player is walking backwards!");
                }
                else if (event == 4) {
                    isSlowedDown = true;
                    std::uniform_int_distribution<> duration(90, 180);
                    slowTimer = duration(gen);
                    m_playerSpeed *= 0.5f;
                    log::info("Player is tired - moving at half speed!");
                }
                else if (event == 5) {
                    hasWeakJump = true;
                    std::uniform_int_distribution<> duration(90, 150);
                    weakJumpTimer = duration(gen);
                    log::info("Player has low energy - weak jumps!");
                }
                else if (event == 6) {
                    isStuttering = true;
                    std::uniform_int_distribution<> duration(90, 150);
                    stutterTimer = duration(gen);
                    stutterState = 0;
                    log::info("Player is stuttering - unstable movement!");
                }
                else if (event == 7) {
                    isSleeping = true;
                    std::uniform_int_distribution<> duration(120, 180);
                    sleepTimer = duration(gen);
                    m_playerSpeed *= 0.3f;
                    log::info("Player fell asleep - barely moving!");
                }
                else if (event == 8) {
                    isFalling = true;
                    std::uniform_int_distribution<> duration(40, 80);
                    fallTimer = duration(gen);
                    log::info("Player is stumbling - random falls!");
                }
                else if (event == 9) {
                    isHeavy = true;
                    std::uniform_int_distribution<> duration(90, 150);
                    heavyTimer = duration(gen);
                    log::info("Player feels heavy - stronger gravity!");
                }
                else if (event == 10) {
                    isDistracted = true;
                    std::uniform_int_distribution<> duration(120, 180);
                    distractTimer = duration(gen);
                    distractCounter = 0;
                    log::info("Player is distracted - not paying attention!");
                }

                frameCounter = 0;
            }
        }

        if (isLazy) {
            lazyTimer--;
            if (lazyTimer <= 0) {
                isLazy = false;
                log::info("Player is no longer lazy");
            }
        }

        if (movingBackwards) {
            m_playerSpeed = -m_playerSpeed;
            backwardsTimer--;
            if (backwardsTimer <= 0) {
                movingBackwards = false;
                m_playerSpeed = -m_playerSpeed;
                log::info("Player stopped walking backwards");
            }
        }

        if (isSlowedDown) {
            slowTimer--;
            if (slowTimer <= 0) {
                isSlowedDown = false;
                m_playerSpeed *= 2.0f;
                log::info("Player is no longer tired");
            }
        }

        if (isStuttering) {
            stutterState++;
            if (stutterState % 20 < 10) {
                m_playerSpeed = 0.0f;
            } else {
                if (m_playerSpeed == 0.0f) {
                    m_playerSpeed = 0.9f;
                }
            }

            stutterTimer--;
            if (stutterTimer <= 0) {
                isStuttering = false;
                stutterState = 0;
                if (m_playerSpeed == 0.0f) {
                    m_playerSpeed = 0.9f;
                }
                log::info("Player stopped stuttering");
            }
        }

        if (hasWeakJump) {
            weakJumpTimer--;
            if (weakJumpTimer <= 0) {
                hasWeakJump = false;
                log::info("Player regained energy");
            }
        }

        if (isSleeping) {
            sleepTimer--;
            if (sleepTimer <= 0) {
                isSleeping = false;
                m_playerSpeed /= 0.3f;
                log::info("Player woke up!");
            }
        }

        if (isFalling) {
            fallTimer--;
            if (fallTimer % 15 == 0) {
                m_yVelocity -= 5.0f;
                log::info("Player stumbled!");
            }
            if (fallTimer <= 0) {
                isFalling = false;
                log::info("Player stopped stumbling");
            }
        }

        if (isHeavy) {
            m_yVelocity -= 0.8f;
            heavyTimer--;
            if (heavyTimer <= 0) {
                isHeavy = false;
                log::info("Player feels lighter now");
            }
        }

        if (isDistracted) {
            distractTimer--;
            if (distractTimer <= 0) {
                isDistracted = false;
                distractCounter = 0;
                log::info("Player is focused again");
            }
        }
    }

    void pushButton(PlayerButton button) {
        if (isModEnabled() && isLazy) {
            log::info("Controls blocked - player fainted!");
            return;
        }

        if (isModEnabled() && isSleeping && button == PlayerButton::Jump) {
            std::uniform_int_distribution<> sleepChance(1, 100);
            if (sleepChance(gen) <= 50) {
                log::info("Jump ignored - player is sleeping!");
                return;
            }
        }

        if (isModEnabled() && isDistracted && button == PlayerButton::Jump) {
            distractCounter++;
            if (distractCounter % 3 == 0) {
                log::info("Jump missed - player is distracted!");
                return;
            }
        }

        if (isModEnabled() && hasWeakJump && button == PlayerButton::Jump) {
            PlayerObject::pushButton(button);
            m_yVelocity *= 0.6f;
            log::info("Weak jump!");
            return;
        }

        PlayerObject::pushButton(button);
    }

    void releaseButton(PlayerButton button) {
        if (isModEnabled() && isLazy) {
            return;
        }

        PlayerObject::releaseButton(button);
    }

    void resetObject() {
        PlayerObject::resetObject();

        frameCounter = 0;
        isLazy = false;
        lazyTimer = 0;
        movingBackwards = false;
        backwardsTimer = 0;
        isSlowedDown = false;
        slowTimer = 0;
        hasWeakJump = false;
        weakJumpTimer = 0;
        isStuttering = false;
        stutterTimer = 0;
        stutterState = 0;
        isSleeping = false;
        sleepTimer = 0;
        isFalling = false;
        fallTimer = 0;
        isHeavy = false;
        heavyTimer = 0;
        isDistracted = false;
        distractTimer = 0;
        distractCounter = 0;
    }
};
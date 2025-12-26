#include "devices/interrupt_controller.hpp"
#include <algorithm>

InterruptController::InterruptController() {
}

void InterruptController::registerSource(std::shared_ptr<InterruptSource> source) {
    if (source) {
        sources.push_back(source);
    }
}

void InterruptController::unregisterSource(std::shared_ptr<InterruptSource> source) {
    sources.erase(std::remove(sources.begin(), sources.end(), source), sources.end());
}

bool InterruptController::hasIRQ() const {
    for (const auto& source : sources) {
        if (source && source->hasIRQ()) {
            return true;
        }
    }
    return false;
}

bool InterruptController::hasNMI() const {
    for (const auto& source : sources) {
        if (source && source->hasNMI()) {
            return true;
        }
    }
    return false;
}

void InterruptController::acknowledgeIRQ() {
    for (auto& source : sources) {
        if (source && source->hasIRQ()) {
            source->clearIRQ();
        }
    }
}

void InterruptController::acknowledgeNMI() {
    for (auto& source : sources) {
        if (source && source->hasNMI()) {
            source->clearNMI();
        }
    }
}

void InterruptController::clearAll() {
    acknowledgeIRQ();
    acknowledgeNMI();
}

size_t InterruptController::getSourceCount() const {
    return sources.size();
}

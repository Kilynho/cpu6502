#include "devices/basic_timer.hpp"
#include <algorithm>

BasicTimer::BasicTimer()
    : counter(0),
      limit(0),
      control(0),
      enabled(false),
      irqEnabled(false),
      irqPending(false),
      autoReload(false),
      limitReached(false),
      initialized(false) {
}

BasicTimer::~BasicTimer() {
    cleanup();
}

bool BasicTimer::initialize() {
    if (initialized) {
        return true;
    }
    
    // Reiniciar todos los registros
    counter = 0;
    limit = 0;
    control = 0;
    enabled = false;
    irqEnabled = false;
    irqPending = false;
    autoReload = false;
    limitReached = false;
    
    initialized = true;
    return true;
}

void BasicTimer::cleanup() {
    if (!initialized) {
        return;
    }
    
    // Deshabilitar timer
    enabled = false;
    irqEnabled = false;
    irqPending = false;
    
    initialized = false;
}

bool BasicTimer::handlesRead(uint16_t address) const {
    return (address >= COUNTER_LOW_ADDR && address <= STATUS_ADDR);
}

bool BasicTimer::handlesWrite(uint16_t address) const {
    return (address >= COUNTER_LOW_ADDR && address <= CONTROL_ADDR);
}

uint8_t BasicTimer::read(uint16_t address) {
    std::lock_guard<std::mutex> lock(timerMutex);
    
    uint32_t currentCounter = counter.load();
    uint32_t currentLimit = limit.load();
    
    switch (address) {
        case COUNTER_LOW_ADDR:
            return static_cast<uint8_t>(currentCounter & 0xFF);
        case COUNTER_MID1_ADDR:
            return static_cast<uint8_t>((currentCounter >> 8) & 0xFF);
        case COUNTER_MID2_ADDR:
            return static_cast<uint8_t>((currentCounter >> 16) & 0xFF);
        case COUNTER_HIGH_ADDR:
            return static_cast<uint8_t>((currentCounter >> 24) & 0xFF);
        case LIMIT_LOW_ADDR:
            return static_cast<uint8_t>(currentLimit & 0xFF);
        case LIMIT_MID1_ADDR:
            return static_cast<uint8_t>((currentLimit >> 8) & 0xFF);
        case LIMIT_MID2_ADDR:
            return static_cast<uint8_t>((currentLimit >> 16) & 0xFF);
        case LIMIT_HIGH_ADDR:
            return static_cast<uint8_t>((currentLimit >> 24) & 0xFF);
        case CONTROL_ADDR:
            return control.load();
        case STATUS_ADDR:
            return getStatusRegister();
        default:
            return 0;
    }
}

void BasicTimer::write(uint16_t address, uint8_t value) {
    std::lock_guard<std::mutex> lock(timerMutex);
    
    uint32_t currentCounter = counter.load();
    uint32_t currentLimit = limit.load();
    
    switch (address) {
        case COUNTER_LOW_ADDR:
            currentCounter = (currentCounter & 0xFFFFFF00) | value;
            counter = currentCounter;
            break;
        case COUNTER_MID1_ADDR:
            currentCounter = (currentCounter & 0xFFFF00FF) | (static_cast<uint32_t>(value) << 8);
            counter = currentCounter;
            break;
        case COUNTER_MID2_ADDR:
            currentCounter = (currentCounter & 0xFF00FFFF) | (static_cast<uint32_t>(value) << 16);
            counter = currentCounter;
            break;
        case COUNTER_HIGH_ADDR:
            currentCounter = (currentCounter & 0x00FFFFFF) | (static_cast<uint32_t>(value) << 24);
            counter = currentCounter;
            break;
        case LIMIT_LOW_ADDR:
            currentLimit = (currentLimit & 0xFFFFFF00) | value;
            limit = currentLimit;
            break;
        case LIMIT_MID1_ADDR:
            currentLimit = (currentLimit & 0xFFFF00FF) | (static_cast<uint32_t>(value) << 8);
            limit = currentLimit;
            break;
        case LIMIT_MID2_ADDR:
            currentLimit = (currentLimit & 0xFF00FFFF) | (static_cast<uint32_t>(value) << 16);
            limit = currentLimit;
            break;
        case LIMIT_HIGH_ADDR:
            currentLimit = (currentLimit & 0x00FFFFFF) | (static_cast<uint32_t>(value) << 24);
            limit = currentLimit;
            break;
        case CONTROL_ADDR:
            updateControlFlags(value);
            break;
    }
}

uint32_t BasicTimer::getCounter() const {
    return counter.load();
}

void BasicTimer::setCounter(uint32_t value) {
    std::lock_guard<std::mutex> lock(timerMutex);
    counter = value;
}

void BasicTimer::reset() {
    std::lock_guard<std::mutex> lock(timerMutex);
    counter = 0;
    irqPending = false;
    limitReached = false;
}

bool BasicTimer::isEnabled() const {
    return enabled.load();
}

void BasicTimer::setEnabled(bool en) {
    std::lock_guard<std::mutex> lock(timerMutex);
    enabled = en;
    
    // Actualizar registro de control
    uint8_t ctrl = control.load();
    if (en) {
        ctrl |= CTRL_ENABLE;
    } else {
        ctrl &= ~CTRL_ENABLE;
    }
    control = ctrl;
}

bool BasicTimer::hasIRQ() const {
    return irqPending.load() && irqEnabled.load();
}

void BasicTimer::clearIRQ() {
    std::lock_guard<std::mutex> lock(timerMutex);
    irqPending = false;
}

bool BasicTimer::hasNMI() const {
    return false;  // BasicTimer no genera NMI
}

void BasicTimer::clearNMI() {
    // BasicTimer no genera NMI, no hay nada que limpiar
}

void BasicTimer::tick(uint32_t cycles) {
    if (!enabled.load()) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(timerMutex);
    
    uint32_t currentCounter = counter.load();
    uint32_t currentLimit = limit.load();
    
    // Incrementar contador
    currentCounter += cycles;
    
    // Verificar si se alcanzó el límite
    if (currentLimit > 0 && currentCounter >= currentLimit) {
        limitReached = true;
        
        // Generar IRQ si está habilitado
        if (irqEnabled.load()) {
            irqPending = true;
        }
        
        // Auto-reload si está habilitado
        if (autoReload.load()) {
            // Mantener el overflow para precisión
            currentCounter = currentCounter - currentLimit;
        } else {
            // Detener en el límite
            currentCounter = currentLimit;
            enabled = false;
            
            // Actualizar registro de control
            uint8_t ctrl = control.load();
            ctrl &= ~CTRL_ENABLE;
            control = ctrl;
        }
    }
    
    counter = currentCounter;
}

uint32_t BasicTimer::getLimit() const {
    return limit.load();
}

void BasicTimer::setLimit(uint32_t value) {
    std::lock_guard<std::mutex> lock(timerMutex);
    limit = value;
}

bool BasicTimer::isIRQEnabled() const {
    return irqEnabled.load();
}

bool BasicTimer::isAutoReload() const {
    return autoReload.load();
}

void BasicTimer::updateControlFlags(uint8_t value) {
    // Guardar el valor del registro de control
    control = value;
    
    // Actualizar flags basado en el valor escrito
    enabled = (value & CTRL_ENABLE) != 0;
    irqEnabled = (value & CTRL_IRQ_ENABLE) != 0;
    autoReload = (value & CTRL_AUTO_RELOAD) != 0;
    
    // Si se escribe el bit de IRQ Flag, limpiar el IRQ
    if (value & CTRL_IRQ_FLAG) {
        irqPending = false;
    }
    
    // Si se escribe el bit de Reset, reiniciar el contador
    if (value & CTRL_RESET) {
        counter = 0;
        limitReached = false;
        irqPending = false;
    }
}

uint8_t BasicTimer::getStatusRegister() const {
    uint8_t status = 0;
    
    if (enabled.load()) {
        status |= STATUS_ENABLED;
    }
    
    if (irqPending.load()) {
        status |= STATUS_IRQ_PENDING;
    }
    
    if (limitReached.load()) {
        status |= STATUS_LIMIT_REACHED;
    }
    
    return status;
}

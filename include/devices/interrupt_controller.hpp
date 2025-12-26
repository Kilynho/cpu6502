#ifndef INTERRUPT_CONTROLLER_HPP
#define INTERRUPT_CONTROLLER_HPP

#include <cstdint>
#include <memory>
#include <vector>
#include <functional>

/**
 * @file interrupt_controller.hpp
 * @brief Sistema centralizado de gestión de interrupciones (IRQ/NMI)
 * 
 * Este controlador permite a los dispositivos registrar y disparar interrupciones
 * de forma centralizada, facilitando la integración entre la CPU y los periféricos.
 */
/**
 * @file interrupt_controller.hpp
 * @brief Centralized interrupt management system (IRQ/NMI)
 *
 * This controller allows devices to register and trigger interrupts
 * in a centralized way, facilitating integration between the CPU and peripherals.
 */

/**
 * @brief Tipos de interrupciones soportadas
 */
enum class InterruptType {
    IRQ,  ///< Standard maskable interrupt (Interrupt Request)
    NMI   ///< Non-maskable interrupt (Non-Maskable Interrupt)
};

/**
 * @brief Interfaz para dispositivos que pueden generar interrupciones
 * 
 * Los dispositivos que implementen esta interfaz podrán registrarse en el
 * controlador de interrupciones y reportar cuando tienen una interrupción pendiente.
 */
/**
 * @brief Interface for devices that can generate interrupts
 *
 * Devices implementing this interface can register with the
 * interrupt controller and report when they have a pending interrupt.
 */
class InterruptSource {
public:
    virtual ~InterruptSource() = default;
    
    /**
     * @brief Verifica si el dispositivo tiene una IRQ pendiente
     * @return true si hay IRQ pendiente, false en caso contrario
     */
    virtual bool hasIRQ() const = 0;
    
    /**
     * @brief Verifica si el dispositivo tiene una NMI pendiente
     * @return true si hay NMI pendiente, false en caso contrario
     */
    virtual bool hasNMI() const = 0;
    
    /**
     * @brief Limpia la bandera de IRQ del dispositivo
     */
    virtual void clearIRQ() = 0;
    
    /**
     * @brief Limpia la bandera de NMI del dispositivo
     */
    virtual void clearNMI() = 0;
};

/**
 * @brief Controlador centralizado de interrupciones para el sistema 6502
 * 
 * El InterruptController gestiona todas las fuentes de interrupciones del sistema,
 * permitiendo que múltiples dispositivos generen IRQ o NMI de forma ordenada.
 * 
 * Características:
 * - Soporte para múltiples fuentes de interrupción
 * - Gestión separada de IRQ y NMI
 * - Prioridad: NMI tiene mayor prioridad que IRQ
 * - IRQ respeta el flag I (Interrupt Disable) de la CPU
 * - NMI no puede ser enmascarado
 * 
 * Ejemplo de uso:
 * @code
 * InterruptController intCtrl;
 * auto timer = std::make_shared<BasicTimer>();
 * 
 * // Registrar el timer como fuente de interrupciones
 * intCtrl.registerSource(timer);
 * 
 * // En el loop principal de la CPU
 * if (intCtrl.hasIRQ() && !cpu.I) {
 *     cpu.serviceIRQ(memory);
 *     intCtrl.acknowledgeIRQ();
 * }
 * @endcode
 * @brief Centralized interrupt controller for the 6502 system
 *
 * The InterruptController manages all interrupt sources in the system,
 * allowing multiple devices to generate IRQ or NMI in an organized way.
 *
 * Features:
 * - Support for multiple interrupt sources
 * - Separate management of IRQ and NMI
 * - Priority: NMI has higher priority than IRQ
 * - IRQ respects the CPU's I (Interrupt Disable) flag
 * - NMI cannot be masked
 *
 * Usage example:
 * @code
 * InterruptController intCtrl;
 * auto timer = std::make_shared<BasicTimer>();
 *
 * // Register the timer as an interrupt source
 * intCtrl.registerSource(timer);
 *
 * // In the main CPU loop
 * if (intCtrl.hasIRQ() && !cpu.I) {
 *     cpu.serviceIRQ(memory);
 *     intCtrl.acknowledgeIRQ();
 * }
 * @endcode
 */

class InterruptController {
public:
    InterruptController();
    ~InterruptController() = default;
    
    /**
     * @brief Registra una fuente de interrupciones
     * @param source Puntero compartido a la fuente de interrupciones
     */
    void registerSource(std::shared_ptr<InterruptSource> source);
    
    /**
     * @brief Elimina una fuente de interrupciones registrada
     * @param source Puntero compartido a la fuente a eliminar
     */
    void unregisterSource(std::shared_ptr<InterruptSource> source);
    
    /**
     * @brief Verifica si hay alguna IRQ pendiente en el sistema
     * @return true si algún dispositivo tiene IRQ pendiente, false en caso contrario
     */
    bool hasIRQ() const;
    
    /**
     * @brief Verifica si hay alguna NMI pendiente en el sistema
     * @return true si algún dispositivo tiene NMI pendiente, false en caso contrario
     */
    bool hasNMI() const;
    
    /**
     * @brief Reconoce y limpia todas las IRQs pendientes
     * 
     * Este método debe ser llamado después de que la CPU haya atendido la interrupción
     * para limpiar las banderas de todos los dispositivos que generaron la IRQ.
     */
    void acknowledgeIRQ();
    
    /**
     * @brief Reconoce y limpia todas las NMIs pendientes
     * 
     * Este método debe ser llamado después de que la CPU haya atendido la interrupción
     * para limpiar las banderas de todos los dispositivos que generaron la NMI.
     */
    void acknowledgeNMI();
    
    /**
     * @brief Limpia todas las interrupciones pendientes (IRQ y NMI)
     */
    void clearAll();
    
    /**
     * @brief Obtiene el número de fuentes de interrupción registradas
     * @return Cantidad de fuentes registradas
     */
    size_t getSourceCount() const;
    
private:
    std::vector<std::shared_ptr<InterruptSource>> sources; ///< Registered interrupt sources
};

#endif // INTERRUPT_CONTROLLER_HPP

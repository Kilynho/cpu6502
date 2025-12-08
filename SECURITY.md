# Política de Seguridad

## Versiones Soportadas

Este proyecto está actualmente en desarrollo activo. Las siguientes versiones reciben actualizaciones de seguridad:

| Versión | Soportada          |
| ------- | ------------------ |
| 0.1.x   | :white_check_mark: |
| < 0.1   | :x:                |

## Reportar una Vulnerabilidad

La seguridad de nuestro proyecto es importante para nosotros. Si descubres una vulnerabilidad de seguridad, te agradecemos que nos lo comuniques de manera responsable.

### Cómo Reportar

**Por favor, NO reportes vulnerabilidades de seguridad a través de issues públicos de GitHub.**

En su lugar, reporta las vulnerabilidades de seguridad a través de:

1. **GitHub Security Advisories** (método preferido):
   - Ve a la pestaña "Security" del repositorio
   - Haz clic en "Report a vulnerability"
   - Completa el formulario con los detalles

2. **Email directo**:
   - Envía un email a los mantenedores del proyecto
   - Incluye "SECURITY" en el asunto
   - Proporciona detalles completos del problema

### Información a Incluir

Por favor, incluye la siguiente información en tu reporte:

- **Tipo de vulnerabilidad** (ej., buffer overflow, inyección, etc.)
- **Ubicación** del código fuente afectado (archivo y número de línea)
- **Configuración especial** requerida para reproducir el problema
- **Pasos detallados** para reproducir la vulnerabilidad
- **Prueba de concepto** o código de explotación (si es posible)
- **Impacto** del problema, incluyendo cómo un atacante podría explotarlo
- **Versión afectada** del software

### Ejemplo de Reporte

```
Asunto: SECURITY - Buffer overflow en función FetchByte

Tipo: Buffer Overflow
Archivo: cpu.cpp, línea 123
Versión afectada: 0.1.0

Descripción:
La función FetchByte no valida correctamente los límites de memoria,
permitiendo leer fuera del rango de memoria asignado.

Pasos para reproducir:
1. Crear un objeto CPU y Mem
2. Configurar PC a 0xFFFF
3. Llamar a FetchByte()
4. Se produce una lectura fuera de límites

Impacto:
Puede permitir la lectura de memoria no autorizada y potencialmente
causar un crash del programa.

Sugerencia de solución:
Añadir validación de límites antes de acceder a la memoria.
```

## Proceso de Respuesta

Cuando reportes una vulnerabilidad:

1. **Confirmación** (24-48 horas):
   - Confirmaremos la recepción de tu reporte
   - Te proporcionaremos un timeline estimado para la investigación

2. **Investigación** (1-7 días):
   - Validaremos y reproduciremos la vulnerabilidad
   - Evaluaremos el impacto y severidad
   - Te mantendremos informado del progreso

3. **Desarrollo de Parche** (variable):
   - Desarrollaremos y probaremos una solución
   - Podemos solicitar tu ayuda para validar el fix

4. **Divulgación** (coordinada):
   - Publicaremos el parche en una nueva versión
   - Te acreditaremos en el security advisory (si lo deseas)
   - Publicaremos un CVE si es aplicable

## Timeline de Divulgación

- Intentamos resolver vulnerabilidades críticas en **7 días**
- Vulnerabilidades de severidad media/alta en **30 días**
- Vulnerabilidades de baja severidad en **90 días**

Preferimos la **divulgación coordinada**:
- Te pedimos que mantengas la vulnerabilidad confidencial hasta que publiquemos un fix
- Una vez que el parche esté disponible, puedes divulgar públicamente
- Te notificaremos cuando sea seguro divulgar

## Severidad de Vulnerabilidades

Clasificamos las vulnerabilidades usando el sistema CVSS:

- **Crítica** (9.0-10.0): Explotable remotamente, sin autenticación, alto impacto
- **Alta** (7.0-8.9): Explotable con complejidad baja, impacto significativo
- **Media** (4.0-6.9): Requiere condiciones especiales, impacto moderado
- **Baja** (0.1-3.9): Difícil de explotar, impacto limitado

## Vulnerabilidades Conocidas

Actualmente no hay vulnerabilidades de seguridad conocidas en las versiones soportadas.

### Historial de Vulnerabilidades

No hay vulnerabilidades reportadas o corregidas hasta la fecha.

## Mejores Prácticas de Seguridad

Para usuarios del proyecto:

1. **Mantén actualizado**: Usa siempre la última versión
2. **Valida entradas**: Si cargas programas externos, valida los datos
3. **Limita permisos**: Ejecuta el emulador con los permisos mínimos necesarios
4. **Monitorea recursos**: El emulador no debería consumir recursos excesivos

Para desarrolladores:

1. **Validación de límites**: Siempre valida accesos a memoria
2. **Manejo de errores**: Implementa manejo robusto de errores
3. **Code review**: Revisa el código para vulnerabilidades comunes
4. **Tests de seguridad**: Incluye tests para casos edge y límites

## Alcance de Seguridad

### En Alcance

- Vulnerabilidades en el código del emulador (cpu.cpp, mem.cpp)
- Problemas de memoria (buffer overflow, use-after-free)
- Problemas de lógica que permitan bypass de restricciones
- Vulnerabilidades en el sistema de build

### Fuera de Alcance

- Vulnerabilidades en dependencias externas (reportar al proyecto upstream)
- Problemas en el código de ejemplo o tests
- Issues de usabilidad o bugs que no sean de seguridad
- Ataques de ingeniería social

## Reconocimientos

Agradecemos a los siguientes investigadores de seguridad por sus contribuciones responsables:

*(Ninguno aún - ¡sé el primero!)*

## Recursos Adicionales

- [OWASP Top 10](https://owasp.org/www-project-top-ten/)
- [CWE - Common Weakness Enumeration](https://cwe.mitre.org/)
- [CVE - Common Vulnerabilities and Exposures](https://cve.mitre.org/)
- [GitHub Security Best Practices](https://docs.github.com/en/code-security)

## Contacto

Para asuntos de seguridad urgentes, contacta directamente a los mantenedores del proyecto a través de GitHub.

---

**Última actualización:** Diciembre 2024

¡Gracias por ayudar a mantener seguro el CPU 6502 Emulator!

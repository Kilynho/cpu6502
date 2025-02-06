# Configuración de Dart para CTest y CDash

# Nombre del proyecto
ProjectName = cpu6502

# Dirección del servidor CDash
DropSite = cdash.example.com
DropLocation = /submit.php?project=cpu6502
DropMethod = http
DropSiteUser = cdash_user
DropSitePassword = cdash_password

# Configuración de la compilación
BuildName = Linux-gcc
Site = localhost

# Configuración de la prueba
CTestCommand = ctest -D Experimental

# Configuración de la cobertura
CoverageCommand = gcov
CoverageExtraFlags = -lp

# Configuración de la memoria
MemCheckCommand = valgrind
MemCheckCommandOptions = --tool=memcheck --leak-check=full --error-exitcode=1
MemCheckSuppressionFile = /path/to/valgrind.supp

# Configuración de la documentación
DocumentationCommand = doxygen
DocumentationOutput = /path/to/doxygen/output

# Configuración de la notificación
EmailNotification = true
EmailRecipients = developer@example.com
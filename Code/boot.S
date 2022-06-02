# Declarar constantes utilizadas para crear un encabezado de multiboot.

.set ALIGN,    1<<0             # Alinear módulos cargados en los limites de las páginas
.set MEMINFO,  1<<1             # Proporciona mapa de memoria
.set FLAGS,    ALIGN | MEMINFO  # Este es el campo bandera del multiboot
.set MAGIC,    0x1BADB002       # Este es el número magico que permite que el multiboot encuentre el encabezado/ averiguar de esto
.set CHECKSUM, -(MAGIC + FLAGS) # suma de comprobación para demostrar que se está en multiboot 

# Declare un encabezado como en el estándar de arranque múltiple. Ponemos esto en una sección
# especial por lo que podemos forzar el que el ecnabezado esté al comienzo del programa final.
# No necesita entender todos estos detalles ya que son valores mágicos que
# estan documentados en el estandar multiboot. El bootloader buscará esta secuencia mágica 
# y reconocerá como un kernel multiboot.

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Actualmente el registro stack pointer(esp) apunnta a cualquiuer cosa y su uso puede
# causar un daño masivo. En su lugar proporcionaremos nuestra propia pila. Nosotros asignamos
# espacio para una pequeña pila temporal creando un simbolo en la parte inferior,
# luego asignándole 16384 bytes y finalmente creando un simbolo en la parte superior

.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# El script linker especifica _start como el punto de entrada al kernel y el
# bootloader saltará a esta posición una vez que se haya cargado el kernel.
# No tiene sentido volver de esta función ya que el gestor de arranque se ha ido.

.section .text
.global _start
.type _start, @function
_start:

	# Bienvenidos al modo Kernel, ahora tenemos suficiente código para que el bootloader
	# cargue y ejecute nuestro OS. Todavía no hace nada interesante.
	# Quizas nos giratría llamar a printf("Hello, World\n"). Deberías ahora 
	# darse cuenta de unas de las profundas verdades del modo kernel: No hay nada ahí
	# a menos de que lo proporcione usted mismo. No hay función printf. No hay
	# cabecera <stdio.h>. Si quieres una función, tendras que codificarlo. Y esa es una de las mejores cosas del desarrollo del Kernel:
	# puedes hacer el sistema tu mismo. tienes absolulto y completo
	# poder sobre la máquina, no hay restricciones de seguridad, 
	# sin salva guardias, sin mecanismos de depuración no hay nada más que lo que construyes.

	# Quizas estes cansado del lenguaje ensamblador. Te das cuentas de algunas
	# cosas simplemente no se pueden hacer en C, como hacer que el encabezado de multiboot
	# en la sección derecha y configurar el stack. Sin embarg, le gustaría
	# escribir el OS en un lenguaje de alto nivel como C o C++.
	# Con ese fin la siguiente tarea es preparar el procesador para la ejecución de
	# tal código. C no espera mucho en este momento y solo necesitamos configurar
	# un stack. Tenga en cuenta que el procesador no está completamente inicializado y todo eso
	# como las instrucciones de coma flotante aun no estan disponibles 

	# Para configurar el stack simplemente coonfiguramos el registro esp para que apunte a la prte superior de 
	# nuestro stack (a medida que crece hacia abajo).

	movl $stack_top, %esp

	# Ahora estamos listos para ejecutar realmente el código C. No podemos colocar eso en un
	# archivo ensamblador, así que creamos un archivo kernel.c. En este archivo,
	# crearemos un punto de entrada en C llamado kernel_main y lo llamaremos aquí.

	call kernel_main

	# En caso de que la función regrese queremos poner la computadora en un
	# ciclo infinito. Para hacer eso usamos lainstrucción "clear interrup" (cli)
	# para deshabilitar las interrupciones, la instrucción de detención ('hlt') para detener la CPU hasta
	# que llega a la siguente interrupción y salta a la instrucción de detención si alguna vez
	# continua la ejecución, solo para estar seguro. Crearemos una etiqueta local en lugar
	# que un simbolo real y saltar allí sin cesar.

	cli
	hlt
	
.Lhang:
	jmp .Lhang

# Establezca el tamaño del simbolo _start en la ubicación actual "." menos su comienzo.
# Esto es util al depurar o al implementar el seguimiento de llamadas.

.size _start, . - _start

# _Ping y CSI_
Este software ha sido desarrollado para poder obtener el CSI (Channel State Information) entre un ESP32 y un router utilizando diferentes protocolos de comunicación y canales disponibles en la banda de 2.4GHz.
El software está diseñado específicamente para una placa diseñada con KiCAD cuya documentación se encuentra en el siguiente enlace:https://github.com/kevingiribuela/Validacion/tree/main/PCB%20-%20V8

Sin embargo, es posible utilizar el mismo software cambiando la definición de 2 pines, el pulsador de propósito general, y el LED. Es muy sencillo dado que se encuentran en las primeras líneas de código del main.c

## ¿Cómo utilizarlo?
A continuación se describe la utilización del código para el caso de que se utilice la placa diseñada en KiCAD V6. Se asume que el usuario ha grabado el software en la placa y ya tiene el dispositivo listo para ser configurado.
1) La placa comenzará a destellar el LED con un período de 300mS indicando que se encuentra en modo AP. En esta situación, el ESP32 genera un webserver en el cuál se le pasa a través de él los datos de la red a la cual se desea que el dispositivo se conecte junto con el protocolo de comunicación a utilizar, a saber 802.11b/bg/bgn.
<img src="https://github.com/kevingiribuela/PING-CSI/blob/master/Imagenes/redes.jpg?raw=true" width=25% height=25%>
2) Ingresar con un celular, tablet, notebook a la red "WIMUMO" con la clave "defiunlp2022".
<img src="https://github.com/kevingiribuela/PING-CSI/blob/master/Imagenes/conectado.jpg?raw=true" width=25% height=25%>

4) Una vez conectado al dispositivo, se deberá ingresar a la siguiente dirección IP con cualquier navegador: 192.168.4.1
5) Dentro del webserver aparecerá la siguiente imagen, en ella se deben ingresar los datos dela red a la cual se desea que el dispositivo se conecte. 
<img src="https://github.com/kevingiribuela/PING-CSI/blob/master/Imagenes/ingreso%20de%20datos.jpg?raw=true" width=25% height=25%>
6) Una vez ingresado los datos, se debe presionar el boton "Enviar datos" y emergerá el siguiente aviso indicando que los datos se han cargados y que el dispositivo se configurará en modo STA para conectarse a la red pasada a través del webserver.
<img src="https://github.com/kevingiribuela/PING-CSI/blob/master/Imagenes/listo.jpg?raw=true" width=25% height=25%>
7) En esta situación, si todo salió bien el dispositivo destellará rápidamente el LED indicando que obtuvo una dirección IP y está conectado al AP. 
8) Ahora para poder obtener información acerca del canal de comunicación simplemente se deberá presionar el boton de propósito general y automáticamente se mostrará por el puerto serie información acerca del CSI durante 10 segundos aproximadamente.
<img src="https://github.com/kevingiribuela/PING-CSI/blob/master/Imagenes/consola.jpg?raw=true">


## Algunos resultados del patrón de radiación
Este software se ha utilizado para relevar de manera experimental el patrón de radiación de la antena del ESP32. Utilizando otros sofwares de simulación se pudo obtener el patrón de radiación en 3D de la antena. 
<img src="https://github.com/kevingiribuela/PING-CSI/blob/master/Imagenes/plano_xy.png?raw=true">
<img src="https://github.com/kevingiribuela/PING-CSI/blob/master/Imagenes/plano_xz.png?raw=true">
<img src="https://github.com/kevingiribuela/PING-CSI/blob/master/Imagenes/plano_yz.png?raw=true">

Si bien las imagenes anterior no se obtuvieron con el software previo, da una idea de lo que se espera al tomar muestras. A continuación se muestra un diagrama polar de las diferentes ganancias obtenidas en función de la polarización de la antena del ESP32 con un router, para obtener una idea coherente de los datos medidos, se realiza una interpolación polinómica.

<img src="https://github.com/kevingiribuela/PING-CSI/blob/master/Imagenes/esp32b.png?raw=true">
<img src="https://github.com/kevingiribuela/PING-CSI/blob/master/Imagenes/esp32.png?raw=true">

